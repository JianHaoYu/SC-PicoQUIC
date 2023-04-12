#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from sys import argv
import time

class MyTopo( Topo ):
    "Simple topology example."
    def build( self,n=1):
        "Create custom topo."
        Host1 = self.addHost( 'h1')
        Host2 = self.addHost( 'h2')
        Switch1 = self.addSwitch('s1')
        self.addLink( Host1, Switch1, bw=500, delay='25ms')
        Switch2 = self.addSwitch('s2')
        self.addLink( Host2, Switch2, bw=500, delay='25ms')
        # Add links
        # bottleneck link
        # 20Mbps, 100ms delay, 1% packet loss
        self.addLink( Switch1, Switch2, bw=25, delay='300ms', loss=0.1)
        # self.addLink( Switch1, Switch2, bw=25, delay='250ms', loss=1)



def perfTest(num=2,i=0):
    "Create network and run simple performance test"
    #topo = SingleSwitchTopo( n=4, lossy=lossy )
    topo = MyTopo(n=num)
    net = Mininet( topo=topo,
                   link=TCLink,
                   autoStaticArp=True )
    
    nodes = net.switches 

    net.start()
    print ("Testing network connectivity")
    info( "Dumping host connections\n" )
    dumpNodeConnections(net.hosts)
    dumpNodeConnections(net.switches)
    h = net.get('h1')
    h.cmd('cd /home/mininet/picoquic/;./picoquicdemo -G cubic >s_PICOQUIC_Satellite_CUBIC_01_25000_'+str(i)+'.log &')
    h = net.get('h2')
    h.cmd('cd /home/mininet/picoquic/;./picoquicdemo -G cubic -n 10.0.0.1 10.0.0.1 4443 >c_PICOQUIC_Satellite_CUBIC_01_25000_'+str(i)+'.log')
    print("test "+str(i)+" over ")
    h = net.get('h1')
    h.cmd('cd /home/mininet/picoquic/;./picoquicdemo >s_PICOQUIC_Satellite_BBR_01_25000_'+str(i)+'.log &')
    h = net.get('h2')
    h.cmd('cd /home/mininet/picoquic/;./picoquicdemo -n 10.0.0.1 10.0.0.1 4443 >c_PICOQUIC_Satellite_BBR_01_25000_'+str(i)+'.log')
    print("test "+str(i)+" over ")
    
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    # Prevent test_simpleperf from failing due to packet loss
    for i in range(0,1):
        perfTest(int(argv[1]),i)
