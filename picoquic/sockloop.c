/*
* Author: Christian Huitema
* Copyright (c) 2020, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Socket loop implements the "wait for messages" loop common to most servers
 * and many clients.
 *
 * Second step: support simple servers and simple client.
 *
 * The "call loop back" function is called: when ready, after receiving, and after sending. The
 * loop will terminate if the callback return code is not zero -- except for special processing
 * of the migration testing code.
 * TODO: in Windows, use WSA asynchronous calls instead of sendmsg, allowing for multiple parallel sends.
 * TODO: in Linux, use multiple send per call API
 * TDOO: trim the #define list.
 * TODO: support the QuicDoq scenario, manage extra socket.
 */

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <iphlpapi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>

#ifndef SOCKET_TYPE
#define SOCKET_TYPE SOCKET
#endif
#ifndef SOCKET_CLOSE
#define SOCKET_CLOSE(x) closesocket(x)
#endif
#ifndef WSA_LAST_ERROR
#define WSA_LAST_ERROR(x) WSAGetLastError()
#endif
#ifndef socklen_t
#define socklen_t int
#endif

#else /* Linux */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>

#ifndef SOCKET_TYPE
#define SOCKET_TYPE int
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_CLOSE
#define SOCKET_CLOSE(x) close(x)
#endif
#ifndef WSA_LAST_ERROR
#define WSA_LAST_ERROR(x) ((long)(x))
#endif
#endif

#include "picosocks.h"
#include "picoquic.h"
#include "picoquic_internal.h"
#include "picoquic_packet_loop.h"
#include "picoquic_unified_log.h"
#include "streamcodec.h"//SC

#if defined(_WINDOWS)
static int udp_gso_available = 0;
#else
# if defined(UDP_SEGMENT)
static int udp_gso_available = 1;
#else
static int udp_gso_available = 0;
#endif
#endif

int picoquic_packet_loop_open_sockets(int local_port, int local_af, SOCKET_TYPE * s_socket, int * sock_af, 
    uint16_t * sock_ports, int socket_buffer_size, int nb_sockets_max)
{
    int nb_sockets = (local_af == AF_UNSPEC) ? 2 : 1;

    /* Compute how many sockets are necessary */
    if (nb_sockets > nb_sockets_max) {
        DBG_PRINTF("Cannot open %d sockets, max set to %d\n", nb_sockets, nb_sockets_max);
        nb_sockets = 0;
    } else if (local_af == AF_UNSPEC) {
        sock_af[0] = AF_INET;
        sock_af[1] = AF_INET6;
    }
    else if (local_af == AF_INET || local_af == AF_INET6) {
        sock_af[0] = local_af;
    }
    else {
        DBG_PRINTF("Cannot open socket(AF=%d), unsupported AF\n", local_af);
        nb_sockets = 0;
    }

    for (int i = 0; i < nb_sockets; i++) {
        struct sockaddr_storage local_address;
        int recv_set = 0;
        int send_set = 0;
        
        if ((s_socket[i] = socket(sock_af[i], SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET ||
            picoquic_socket_set_ecn_options(s_socket[i], sock_af[i], &recv_set, &send_set) != 0 ||
            picoquic_socket_set_pkt_info(s_socket[i], sock_af[i]) != 0 ||
            picoquic_bind_to_port(s_socket[i], sock_af[i], local_port) != 0 ||
            picoquic_get_local_address(s_socket[i], &local_address) != 0 ||
            picoquic_socket_set_pmtud_options(s_socket[i], sock_af[i]) != 0)
        {
            DBG_PRINTF("Cannot set socket (af=%d, port = %d)\n", sock_af[i], local_port);
            for (int j = 0; j < i; j++) {
                if (s_socket[i] != INVALID_SOCKET) {
                    SOCKET_CLOSE(s_socket[i]);
                    s_socket[i] = INVALID_SOCKET;
                }
            }
            nb_sockets = 0;
            break;
        }
        else {
            if (local_address.ss_family == AF_INET6) {
                sock_ports[i] = ntohs(((struct sockaddr_in6*)&local_address)->sin6_port);
            }
            else if (local_address.ss_family == AF_INET) {
                sock_ports[i] = ntohs(((struct sockaddr_in*)&local_address)->sin_port);
            }

            if (socket_buffer_size > 0) {
                socklen_t opt_len;
                int opt_ret;
                int so_sndbuf;
                int so_rcvbuf;

                opt_len = sizeof(int);
                so_sndbuf = socket_buffer_size;
                opt_ret = setsockopt(s_socket[i], SOL_SOCKET, SO_SNDBUF, (const char*)&so_sndbuf, opt_len);
                if (opt_ret != 0) {
#ifdef _WINDOWS
                    int sock_error = WSAGetLastError();
#else
                    int sock_error = errno;
#endif
                    opt_ret = getsockopt(s_socket[i], SOL_SOCKET, SO_SNDBUF, (char*)&so_sndbuf, &opt_len);
                    DBG_PRINTF("Cannot set SO_SNDBUF to %d, err=%d, so_sndbuf=%d (%d)",
                        socket_buffer_size, sock_error, so_sndbuf, opt_ret);
                }
                opt_len = sizeof(int);
                so_rcvbuf = socket_buffer_size;
                opt_ret = setsockopt(s_socket[i], SOL_SOCKET, SO_RCVBUF, (const char*)&so_rcvbuf, opt_len);
                if (opt_ret != 0) {
#ifdef _WINDOWS
                    int sock_error = WSAGetLastError();
#else
                    int sock_error = errno;
#endif
                    opt_ret = getsockopt(s_socket[i], SOL_SOCKET, SO_RCVBUF, (char*)&so_rcvbuf, &opt_len);
                    DBG_PRINTF("Cannot set SO_RCVBUF to %d, err=%d, so_rcvbuf=%d (%d)",
                        socket_buffer_size, sock_error, so_rcvbuf, opt_ret);
                }
            }
        }
    }

    return nb_sockets;
}

int picoquic_packet_loop(picoquic_quic_t* quic,
    int local_port,
    int local_af,
    int dest_if,
    int socket_buffer_size,
    int do_not_use_gso,
    picoquic_packet_loop_cb_fn loop_callback,
    void* loop_callback_ctx)
{
    int ret = 0;
    uint64_t current_time = picoquic_get_quic_time(quic);
    int64_t delay_max = 10000000;
    struct sockaddr_storage addr_from;
    struct sockaddr_storage addr_to;
    int if_index_to;
    uint8_t buffer[1536];
    uint8_t* send_buffer = NULL;
    size_t send_length = 0;
    size_t send_msg_size = 0;
    size_t send_buffer_size = 1536;
    size_t* send_msg_ptr = NULL;
    int bytes_recv;
    picoquic_connection_id_t log_cid;
    SOCKET_TYPE s_socket[PICOQUIC_PACKET_LOOP_SOCKETS_MAX];
    int sock_af[PICOQUIC_PACKET_LOOP_SOCKETS_MAX];
    uint16_t sock_ports[PICOQUIC_PACKET_LOOP_SOCKETS_MAX];
    int nb_sockets = 0;
    int testing_migration = 0; /* Hook for the migration test */
    uint16_t next_port = 0; /* Data for the migration test */
    picoquic_cnx_t* last_cnx = NULL;
    int loop_immediate = 0;
    picoquic_packet_loop_options_t options = { 0 };
    uint64_t next_send_time = current_time + PICOQUIC_PACKET_LOOP_SEND_DELAY_MAX;
    //SC

    size_t SCEnqueuePacketSize =1414;//quic 1410+usefulLength(int)
    size_t SCPacketSize =1430;//quic 1410+usefulLength(int)+scheader(4*int)
    size_t SendPacketSize =1434;//quic 1410+usefulLength(int)+scheader(4*int)+checknum(int)


    uint8_t* enc_send_buffer = NULL;
    size_t enc_send_buffer_size = send_buffer_size;
    uint8_t* header_send_buffer = NULL;        
    int ecno_send_socket=0;
    struct sockaddr_storage enco_addr_dest;
    struct sockaddr_storage enco_addr_from;
    int ecno_dest_if;
    int ecno_callback=0;
    uint8_t* dec_ack_buffer = NULL;
    // uint8_t* recovered_packet_buffer= NULL;
    // size_t recovered_buffer_size = 1536;


    struct parameters cp;
    cp.gfpower = 8;
    cp.pktsize = 1414;
    cp.repfreq = 0.2;
    cp.seed=0;
    struct encoder *ec = initialize_encoder(&cp, NULL, 0);
    struct decoder *dc = initialize_decoder(&cp);

    int SCPacketID =0;
    double SendSourcePacketnum=0;
    double SendRepairPacketnum=0;
    double target_f=0.06;

    int ServerLastGetinorder = -1;
    int ServerLastGetRepairID = -1;

    int LastGetSourceID = -1;
    int LastGetRepairID = -1;
    int LostPacket[5000];
    int LostPacketPointer =0;
    int LostPacketsum =0;
    int Lastinorder=-1;
    float LostRate =0;
    int old_state =0;
    int ClientorServer = 0;//0ForClient;1ForServer;
    if (local_port!=0){//server
        ClientorServer=1;
    }
    if (local_af!=0){//client
        ClientorServer=0;
    }
    //SC

#ifdef _WINDOWS
    WSADATA wsaData = { 0 };
    (void)WSA_START(MAKEWORD(2, 2), &wsaData);
#endif
    memset(sock_af, 0, sizeof(sock_af));
    memset(sock_ports, 0, sizeof(sock_ports));

    if ((nb_sockets = picoquic_packet_loop_open_sockets(local_port, local_af, s_socket, sock_af, 
        sock_ports, socket_buffer_size, PICOQUIC_PACKET_LOOP_SOCKETS_MAX)) == 0) {
        ret = PICOQUIC_ERROR_UNEXPECTED_ERROR;
    }
    else if (loop_callback != NULL) {
        struct sockaddr_storage l_addr;
        ret = loop_callback(quic, picoquic_packet_loop_ready, loop_callback_ctx, &options);

        if (picoquic_store_loopback_addr(&l_addr, sock_af[0], sock_ports[0]) == 0) {
            ret = loop_callback(quic, picoquic_packet_loop_port_update, loop_callback_ctx, &l_addr);
        }
    }

    if (ret == 0) {
        if (udp_gso_available && !do_not_use_gso) {
            printf("USE_UDP_GSO\n");
            send_buffer_size = 0xFFFF;
            enc_send_buffer_size =0xFFFFFF;
            send_msg_ptr = &send_msg_size;
        }
        send_buffer = malloc(send_buffer_size);
        if (send_buffer == NULL) {
            ret = -1;
        }
        header_send_buffer = malloc(SCEnqueuePacketSize);
        if (header_send_buffer == NULL) {
            ret = -1;
        }
        enc_send_buffer = malloc(enc_send_buffer_size);
        if (enc_send_buffer == NULL) {
            ret = -1;
        }
        dec_ack_buffer = malloc(3*sizeof(int)+sizeof(float));
        if (dec_ack_buffer == NULL) {
            ret = -1;
        }
        // recovered_packet_buffer = malloc(recovered_buffer_size);
        // if (recovered_packet_buffer == NULL) {
        //     ret = -1;
        // }
    }

    /* Wait for packets */
    /* TODO: add stopping condition, was && (!just_once || !connection_done) */
    while (ret == 0) {
        int socket_rank = -1;
        int64_t delta_t = 0;
        unsigned char received_ecn;

        if_index_to = 0;
        /* TODO: rewrite the code and avoid using the "loop_immediate" state variable */
        if (!loop_immediate) {
            delta_t = picoquic_get_next_wake_delay(quic, current_time, delay_max);
            if (options.do_time_check) {
                packet_loop_time_check_arg_t time_check_arg;
                time_check_arg.current_time = current_time;
                time_check_arg.delta_t = delta_t;
                ret = loop_callback(quic, picoquic_packet_loop_time_check, loop_callback_ctx, &time_check_arg);
                if (time_check_arg.delta_t < delta_t) {
                    delta_t = time_check_arg.delta_t;
                }
            }
        }
        loop_immediate = 0;

        bytes_recv = picoquic_select_ex(s_socket, nb_sockets,
            &addr_from,
            &addr_to, &if_index_to, &received_ecn,
            buffer, sizeof(buffer),
            delta_t, &socket_rank, &current_time);
        if (bytes_recv < 0) {
            ret = -1;
        }
        else {
            uint64_t loop_time = current_time;

            int b=0;
            if (bytes_recv>0){//SCPacket
                memcpy(&b,buffer,sizeof(int));
                printf("bytes_recv:%d ,b: %d \n",bytes_recv,b);
            }            
            if (bytes_recv>0 && b==3 && ClientorServer==0){//SCPacket
                int StartUsefulLength =0;
                memcpy(&StartUsefulLength,buffer+5*sizeof(int),sizeof(int));
                printf("StartUsefulLength: %d \n",StartUsefulLength);
                memmove(buffer,buffer+sizeof(int),bytes_recv-sizeof(int));//Remove Check
                struct packet *rpkt = deserialize_packet(dc, buffer);          
                receive_packet(dc, rpkt);
                
                int sourceID = rpkt->sourceid;
                int repairID = rpkt->repairid;
                int new_state = dc->active;
                 
                if (sourceID != -1) {//Get Source Packet
		            printf("GETSourcePacket: %d \n", sourceID);
                    if (LastGetSourceID+1 != sourceID) {//Packet Lost
                        while (LastGetSourceID+1 < sourceID){
                            LastGetSourceID=LastGetSourceID+1;
                            LostPacket[LostPacketPointer]=LastGetSourceID;
                            LostPacketPointer=LostPacketPointer+1;
                            LostPacketsum=LostPacketsum+1;
                            // printf("LastGetSourceID:%d\n",LastGetSourceID);
                        }
                    }
                    LastGetSourceID = sourceID;
                    LostRate = (LostPacketsum/1.0)/((sourceID+1)/1.0);
                    // printf("LostPacketsum: %d ,LostRate: %f \n",LostPacketsum,LostRate);

	            }else{//Get Repair Packet
                    LastGetRepairID=repairID;
		            printf("GETRepairPacket: %d \n", repairID);
                    continue;
                }
                memcpy(&StartUsefulLength,buffer+4*sizeof(int),sizeof(int));
                printf("EndUsefulLength: %d \n",StartUsefulLength);                
                int DW =dc->win_e-dc->win_s;
                printf("[DW]: %d ,[Inorder]: %d,[Active]: %d ,[SourceID]: %d \n",DW,dc->inorder,dc->active,sourceID);
                printf("old_state: %d ,new_state: %d ,LostPacketPointer: %d ,LostPacketsum: %d ,LostRate: %f \n",old_state,new_state,LostPacketPointer,LostPacketsum,LostRate);
                if (old_state == 1 && new_state == 0) {//recover Lost packet after state change
                    int i =0;
                    for (i = 0; i < LostPacketPointer; i++) {
                        int LostSourceID = LostPacket[i];
                        uint8_t* recovered_packet_buffer = recover_packet(dc, LostSourceID);
                        // recovered_packet_buffer = recover_packet(dc, LostSourceID);
                        int useful_length =0;
                        memcpy(&useful_length,recovered_packet_buffer,sizeof(int));
                        memmove (recovered_packet_buffer,recovered_packet_buffer+sizeof(int),useful_length);
                        printf("LostSourceID: %d useful_length: %d \n",LostSourceID,useful_length);
                        (void)picoquic_incoming_packet_ex(quic, recovered_packet_buffer,
                            (size_t)useful_length, (struct sockaddr*) & addr_from,
                            (struct sockaddr*) & addr_to, if_index_to, received_ecn,
                            &last_cnx, current_time);
                        if (recovered_packet_buffer != NULL) {
                            free(recovered_packet_buffer);
                        }
                    }
                    LostPacketPointer = 0;
                }
                
                old_state=new_state;
                int useful_length =0;
                memcpy(&useful_length,buffer+4*sizeof(int),sizeof(int));
                printf("uesful length: %d \n",useful_length);
                memmove (buffer,buffer+5*sizeof(int),useful_length);
                
                int inorder = dc->inorder;//send inorder (SC_ACK)
                // if (inorder !=Lastinorder && ecno_callback==1){
                if (ecno_callback==1){
                    int a=6;
                    memcpy(dec_ack_buffer,&a,sizeof(int));
                    memcpy(dec_ack_buffer+sizeof(int),&inorder,sizeof(int));
                    memcpy(dec_ack_buffer+2*sizeof(int),&LostRate,sizeof(float));
                    memcpy(dec_ack_buffer+2*sizeof(int)+sizeof(float),&LastGetRepairID,sizeof(int));
                    picoquic_sendmsg(ecno_send_socket,
                        (struct sockaddr*)&enco_addr_dest, (struct sockaddr*)&enco_addr_from, ecno_dest_if,
                        (const char*)dec_ack_buffer, (int)(sizeof(int)*3+sizeof(float)), (int)(sizeof(int)*3+sizeof(float)), &b);
                }
                Lastinorder=inorder;
                bytes_recv=useful_length;
            }else if (bytes_recv>0 && b==6){//Get SC_ACK
                int inorder;
                memcpy(&inorder,buffer+sizeof(int),sizeof(int));
                printf("SC inorder: %d \n",inorder);
                flush_acked_packets(ec,inorder);
                ServerLastGetinorder = inorder;

                //Update target F
                float GETAckLostRate =0;
                memcpy(&GETAckLostRate,buffer+2*sizeof(int),sizeof(float));
                int GetRepairID=0;
                memcpy(&GetRepairID,buffer+2*sizeof(int)+sizeof(float),sizeof(int));
                ServerLastGetRepairID=GetRepairID;

                if (GETAckLostRate!=0){
                    float currentF =(SendRepairPacketnum)/(SendSourcePacketnum+SendRepairPacketnum);
                    target_f = GETAckLostRate*2.5;
                    // target_f = GETAckLostRate*2+0.01;
                    if (target_f>0.5){
                        target_f=0.5;
                    }
                    float trueF =(SendRepairPacketnum-ServerLastGetRepairID)/(SendSourcePacketnum-ServerLastGetinorder+SendRepairPacketnum-ServerLastGetRepairID);
                    printf("GETACKLostRate: %f ,UpdatetargetF: %f ,currentF: %f ,trueF: %f \n",GETAckLostRate,target_f,currentF,trueF);                    
                }
                continue;
            }

            if (bytes_recv > 0) {
                uint16_t current_recv_port = 0;

                if (testing_migration && socket_rank == 0) {
                    current_recv_port = next_port;
                } else {
                    current_recv_port = sock_ports[socket_rank];
                }
                /* Document incoming port */
                if (addr_to.ss_family == AF_INET6) {
                    ((struct sockaddr_in6*) & addr_to)->sin6_port = current_recv_port;
                }
                else if (addr_to.ss_family == AF_INET) {
                    ((struct sockaddr_in*) & addr_to)->sin_port = current_recv_port;
                }
                /* Submit the packet to the server */
                (void)picoquic_incoming_packet_ex(quic, buffer,
                    (size_t)bytes_recv, (struct sockaddr*) & addr_from,
                    (struct sockaddr*) & addr_to, if_index_to, received_ecn,
                    &last_cnx, current_time);

                if (loop_callback != NULL) {
                    size_t b_recvd = (size_t)bytes_recv;
                    ret = loop_callback(quic, picoquic_packet_loop_after_receive, loop_callback_ctx, &b_recvd);
                }
                if (ret == 0 && current_time < next_send_time) {
                    /* Try to receive more packets if possible */
                    loop_immediate = 1;
                    continue;
                }
                else {
                    next_send_time = current_time + PICOQUIC_PACKET_LOOP_SEND_DELAY_MAX;
                }
            }
            if (ret != PICOQUIC_NO_ERROR_SIMULATE_NAT && ret != PICOQUIC_NO_ERROR_SIMULATE_MIGRATION) {
                size_t bytes_sent = 0;

                while (ret == 0) {
                    struct sockaddr_storage peer_addr;
                    struct sockaddr_storage local_addr;
                    int if_index = dest_if;
                    int sock_ret = 0;
                    int sock_err = 0;

                    ret = picoquic_prepare_next_packet_ex(quic, loop_time,
                        send_buffer, send_buffer_size, &send_length,
                        &peer_addr, &local_addr, &if_index, &log_cid, &last_cnx,
                        send_msg_ptr);
                    if (ret == 0 && send_length > 0) {
                        printf("[Before] (int)send_length: %d ,(int)send_msg_size: %d \n",(int)send_length,(int)send_msg_size);
                        if (ClientorServer==1){
                            size_t enc_Packet_length=0;
                            size_t copyed_Packet_length=0;
                            int enqueue_packet_num=0;
                            int intsend_length =(int)send_length;
                            int intsend_msg_size=(int)send_msg_size;
                            while (intsend_length>0)//enqueue Packet
                            {
                                //Usefull Length
                                int usefule_length =0;
                                if (intsend_length < intsend_msg_size){
                                    usefule_length = intsend_length;
                                }else{
                                    usefule_length = intsend_msg_size;
                                }
                                memcpy(header_send_buffer,&usefule_length,sizeof(int));
                                //send_buffer
                                memcpy(header_send_buffer+sizeof(int),send_buffer+copyed_Packet_length,send_msg_size);
                                //enqueue Packet
                                enqueue_packet(ec, SCPacketID, header_send_buffer);
                                SCPacketID++;
                                enqueue_packet_num++;

                                copyed_Packet_length = copyed_Packet_length+send_msg_size;
                                intsend_length=intsend_length-intsend_msg_size;
                                printf("Send Usefull Length: %d \n",usefule_length);
                            }
                            while (enqueue_packet_num>0)
                            {
                                SendSourcePacketnum++;
                                enqueue_packet_num--;
                                struct packet *pkt;
                                pkt = output_source_packet(ec);
                                unsigned char *pktstr = serialize_packet(ec, pkt);
                                int checkint = 3;
                                memcpy(enc_send_buffer+enc_Packet_length,&checkint,sizeof(int));
                                memcpy(enc_send_buffer+sizeof(int)+enc_Packet_length,pktstr,SCPacketSize);
                                free_packet(pkt);

                                enc_Packet_length=enc_Packet_length+SendPacketSize;
                                //Send Repair Packet if we needed
                                float truef =(SendRepairPacketnum-ServerLastGetRepairID)/(SendSourcePacketnum-ServerLastGetinorder+SendRepairPacketnum-ServerLastGetRepairID);
                                if ( SendSourcePacketnum > 0 && target_f > truef){
                                    //Send Repair Packet(Must be sent after the Source packet)
                                    SendRepairPacketnum++;
                                    struct packet *pkt;
                                    pkt = output_repair_packet(ec);
                                    unsigned char *pktstr = serialize_packet(ec, pkt);
                                    int checkint = 3;
                                    memcpy(enc_send_buffer+enc_Packet_length,&checkint,sizeof(int));
                                    memcpy(enc_send_buffer+sizeof(int)+enc_Packet_length,pktstr,SCPacketSize);
                                    free_packet(pkt);
                                    enc_Packet_length=enc_Packet_length+SendPacketSize;
                                    printf("Send Repair Packet\n");
                                }
                            }
                            send_length=enc_Packet_length;
                            send_msg_size=SendPacketSize;
                            printf("[After] (int)send_length: %d ,(int)send_msg_size: %d \n",(int)send_length,(int)send_msg_size);
                            printf("SCPacketID:%d EW: [%d ,%d] \n",SCPacketID,ec->nextsid,ec->headsid);
                        }else{
                            memcpy(enc_send_buffer,send_buffer,send_length);//Do nothing if we donot want to send a scpacket
                        }

                        SOCKET_TYPE send_socket = INVALID_SOCKET;
                        bytes_sent += send_length;

                        for (int i = 0; i < nb_sockets; i++) {
                            if (sock_af[i] == peer_addr.ss_family) {
                                send_socket = s_socket[i];
                                break;
                            }
                        }

                        if (send_socket == INVALID_SOCKET) {
                            sock_ret = -1;
                            sock_err = -1;
                        }
                        else {
                            if (testing_migration) {
                                /* This code path is only used in the migration tests */
                                uint16_t send_port = (local_addr.ss_family == AF_INET) ?
                                    ((struct sockaddr_in*)&local_addr)->sin_port :
                                    ((struct sockaddr_in6*)&local_addr)->sin6_port;

                                if (send_port == next_port) {
                                    send_socket = s_socket[nb_sockets - 1];
                                }
                            }

                            sock_ret = picoquic_sendmsg(send_socket,
                                (struct sockaddr*)&peer_addr, (struct sockaddr*)&local_addr, if_index,
                                (const char*)enc_send_buffer, (int)send_length, (int)send_msg_size, &sock_err);
                            
                            //Updata Callback information
                            ecno_send_socket = send_socket;
                            ecno_dest_if = if_index;
                            ecno_callback =1;
                            enco_addr_dest = peer_addr;
                            enco_addr_from = local_addr;

                        }

                        if (sock_ret <= 0) {
                            if (last_cnx == NULL) {
                                picoquic_log_context_free_app_message(quic, &log_cid, "Could not send message to AF_to=%d, AF_from=%d, if=%d, ret=%d, err=%d",
                                    peer_addr.ss_family, local_addr.ss_family, if_index, sock_ret, sock_err);
                            }
                            else {
                                picoquic_log_app_message(last_cnx, "Could not send message to AF_to=%d, AF_from=%d, if=%d, ret=%d, err=%d",
                                    peer_addr.ss_family, local_addr.ss_family, if_index, sock_ret, sock_err);

                                if (picoquic_socket_error_implies_unreachable(sock_err)) {
                                    picoquic_notify_destination_unreachable(last_cnx, current_time,
                                        (struct sockaddr*)&peer_addr, (struct sockaddr*)&local_addr, if_index,
                                        sock_err);
                                }
                                else if (sock_err == EIO) {
                                    size_t packet_index = 0;
                                    size_t packet_size = send_msg_size;

                                    while (packet_index < send_length) {
                                        if (packet_index + packet_size > send_length) {
                                            packet_size = send_length - packet_index;
                                        }
                                        sock_ret = picoquic_sendmsg(send_socket,
                                            (struct sockaddr*)&peer_addr, (struct sockaddr*)&local_addr, if_index,
                                            (const char*)(send_buffer + packet_index), (int)packet_size, 0, &sock_err);
                                        if (sock_ret > 0) {
                                            packet_index += packet_size;
                                        }
                                        else {
                                            picoquic_log_app_message(last_cnx, "Retry with packet size=%zu fails at index %zu, ret=%d, err=%d.",
                                                packet_size, packet_index, sock_ret, sock_err);
                                            break;
                                        }
                                    }
                                    if (sock_ret > 0) {
                                        picoquic_log_app_message(last_cnx, "Retry of %zu bytes by chunks of %zu bytes succeeds.",
                                            send_length, send_msg_size);
                                    }
                                    if (send_msg_ptr != NULL) {
                                        /* Make sure that we do not use GSO anymore in this run */
                                        send_msg_ptr = NULL;
                                        picoquic_log_app_message(last_cnx, "%s", "UDP GSO was disabled");
                                    }
                                }
                            }
                        }
                    }
                    else {
                        break;
                    }
                }

                if (ret == 0 && loop_callback != NULL) {
                    ret = loop_callback(quic, picoquic_packet_loop_after_send, loop_callback_ctx, &bytes_sent);
                }
            }
        }

        if (ret == PICOQUIC_NO_ERROR_SIMULATE_NAT || ret == PICOQUIC_NO_ERROR_SIMULATE_MIGRATION) {
            /* Two pseudo error codes used for testing migration!
             * What follows is really test code, which we write here because it has to handle
             * the sockets, which interferes a lot with the handling of the packet loop.
             */
            SOCKET_TYPE s_mig = INVALID_SOCKET;
            int s_mig_af;
            int sock_ret;
            int testing_nat = (ret == PICOQUIC_NO_ERROR_SIMULATE_NAT);

            sock_ret = picoquic_packet_loop_open_sockets(0, sock_af[0], &s_mig, &s_mig_af,
                &next_port, socket_buffer_size, 1);
            if (sock_ret != 1 || s_mig == INVALID_SOCKET) {
                if (last_cnx != NULL) {
                    picoquic_log_app_message(last_cnx, "Could not create socket for migration test, port=%d, af=%d, err=%d",
                        next_port, sock_af[0], sock_ret);
                }
            }
            else if (testing_nat) {
                if (s_socket[0] != INVALID_SOCKET) {
                    SOCKET_CLOSE(s_socket[0]);
                }
                s_socket[0] = s_mig;
                sock_ports[0] = next_port;
                ret = 0;

                if (loop_callback != NULL) {
                    struct sockaddr_storage l_addr;
                    if (picoquic_store_loopback_addr(&l_addr, sock_af[0], sock_ports[0]) == 0) {
                        ret = loop_callback(quic, picoquic_packet_loop_port_update, loop_callback_ctx, &l_addr);
                    }
                }
            } else {
                /* Testing organized migration */
                if (nb_sockets < PICOQUIC_PACKET_LOOP_SOCKETS_MAX && last_cnx != NULL) {
                    struct sockaddr_storage local_address;
                    picoquic_store_addr(&local_address, (struct sockaddr*)& last_cnx->path[0]->local_addr);
                    if (local_address.ss_family == AF_INET6) {
                        ((struct sockaddr_in6*) & local_address)->sin6_port = next_port;
                    }
                    else if (local_address.ss_family == AF_INET) {
                        ((struct sockaddr_in*) & local_address)->sin_port = next_port;
                    }
                    s_socket[nb_sockets] = s_mig;
                    sock_ports[nb_sockets] = next_port;
                    nb_sockets++;
                    testing_migration = 1;
                    ret = picoquic_probe_new_path(last_cnx, (struct sockaddr*)&last_cnx->path[0]->peer_addr,
                        (struct sockaddr*) &local_address, current_time);
                }
                else {
                    SOCKET_CLOSE(s_mig);
                }
            }
        }
    }

    if (ret == PICOQUIC_NO_ERROR_TERMINATE_PACKET_LOOP) {
        /* Normal termination requested by the application, returns no error */
        ret = 0;
    }

    /* Close the sockets */
    for (int i = 0; i < nb_sockets; i++) {
        if (s_socket[i] != INVALID_SOCKET) {
            SOCKET_CLOSE(s_socket[i]);
            s_socket[i] = INVALID_SOCKET;
        }
    }

    if (send_buffer != NULL) {
        free(send_buffer);
    }
    if (header_send_buffer != NULL) {
        free(header_send_buffer);
    }
    if (enc_send_buffer != NULL) {
        free(enc_send_buffer);
    }
    if (dec_ack_buffer != NULL) {
        free(dec_ack_buffer);
    }
    // if (recovered_packet_buffer != NULL) {
    //     free(recovered_packet_buffer);
    // }
    return ret;
}
