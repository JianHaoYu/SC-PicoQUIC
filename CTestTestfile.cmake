# CMake generated Testfile for 
# Source directory: /home/mininet/picoquic
# Build directory: /home/mininet/picoquic
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(picoquic_ct "/home/mininet/picoquic/picoquic_ct" "-S" "/home/mininet/picoquic" "-n" "-r")
set_tests_properties(picoquic_ct PROPERTIES  _BACKTRACE_TRIPLES "/home/mininet/picoquic/CMakeLists.txt;342;add_test;/home/mininet/picoquic/CMakeLists.txt;0;")
add_test(picohttp_ct "/home/mininet/picoquic/picohttp_ct" "-S" "/home/mininet/picoquic" "-n" "-r" "-x" "http_corrupt")
set_tests_properties(picohttp_ct PROPERTIES  _BACKTRACE_TRIPLES "/home/mininet/picoquic/CMakeLists.txt;344;add_test;/home/mininet/picoquic/CMakeLists.txt;0;")
subdirs("_deps/picotls-build")
