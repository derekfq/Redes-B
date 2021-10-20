#include "NetLayer.h"
#include <stdio.h>	//for printf
#include <stdlib.h> //for exit(0);
//#include <iostream>
#include <algorithm>
#include <string.h> //for memset
#include <sys/socket.h>	//for socket ofcourse
#include <arpa/inet.h> // inet_addr

//-- constructors & destructor -----------------------------------------------------------------------------------------
NetLayer::NetLayer(int proto) { _initialize(proto); }

NetLayer::NetLayer(int proto, unsigned short local_port) { _initialize(proto, local_port); }

NetLayer::NetLayer(int proto, unsigned short local_port, char * spoofed_localAddress) {
    _initialize(proto, local_port, spoofed_localAddress);
}

NetLayer::~NetLayer() { _terminate(); }
//-- functions ---------------------------------------------------------------------------------------------------------
ssize_t NetLayer::SendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port) {
    if(spoofed)
        _spoofed_sendDataTo(buf, len, dest_addr, dest_port);
    else
        _sendDataTo(buf, len, dest_addr, dest_port);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void NetLayer::_initialize(int proto, unsigned short local_port, char * spoof_localAddr) {

}
void NetLayer::_terminate() {

}

ssize_t NetLayer::_sendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port){
    return 0;
}

ssize_t NetLayer::_spoofed_sendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port){
    return 0;
}