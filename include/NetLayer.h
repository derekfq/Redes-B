#pragma once
#include <list>
#include <netinet/ip.h>	    //Provides declarations for ip header
#include <netinet/tcp.h>    //Provides declarations for tcp header
#include <linux/udp.h>      //Provides declarations for udp header
#include <unistd.h> // sleep()
#include <errno.h>          //For errno - the error number

//- defines --------------------------------------------------------------------
typedef struct {
    //network layer (IPv4 headers)
    //transport layer (UDP/TCP) <talvez aqui seja interessante union>
    //application layer (Payload)
} NetPacket;
typedef std::list<NetPacket *> NetPacketList;
typedef unsigned char byte;
//- Classes --------------------------------------------------------------------
class NetLayer {
    //- variables ----
private:
    NetPacketList pckts;
    struct sockaddr_in localaddr; //includes ip and port
    bool spoofed=false;
    int s, r; //sending and receiving sockets.
public:
    //- Constructors -
    /* Ideal para somente envios */
    NetLayer(int proto);
    /* Ideal para receber numa porta fixa */
    NetLayer(int proto, unsigned short local_port);
    /* Ideal para spoofar o endereço  de ip local */ //(Se passar)
    NetLayer(int proto, unsigned short local_port, char * spoofed_localAddress);
    ~NetLayer();
    //- functions ----
    ssize_t SendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    //ssize_t RecvDataFrom(void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    //Getters&Setters
    void GetLocalIpAddress(char * ip, unsigned short size);
    unsigned short GetLocalPort();
private:
    ssize_t _sendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    ssize_t _spoofed_sendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    /* Initializes upon a random local port by default (local_port=0),
       using spoofed_localAddr=NULL by default means no spoofing Local Address. */
    void _initialize(int proto, unsigned short local_port=0, char * spoof_localAddr=NULL);
    /* Terminates class allocations */
    void _terminate();
}; 
//------------------------------------------------------------------------------
