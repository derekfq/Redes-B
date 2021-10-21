#pragma once
#include <list>
#include <netinet/ip.h>	    //Provides declarations for ip header
#include <netinet/tcp.h>    //Provides declarations for tcp header
#include <linux/udp.h>      //Provides declarations for udp header
#include <unistd.h> // sleep()
#include <errno.h>          //For errno - the error number

//- defines --------------------------------------------------------------------
#define MTU 1500

typedef unsigned char byte;
typedef struct {
    //network layer (IPv4 header)
    struct iphdr * iph;
    //transport layer (UDP/TCP)
    struct udphdr * udph;
    struct tcphdr * tcph;
    //application layer (Payload)
    byte datagram[MTU];
} net_packet;
typedef std::list<net_packet*> net_packet_list;
//- Classes --------------------------------------------------------------------
class NetLayer {
    //- variables ----
private:
    struct sockaddr_in localaddr; //includes ip and port
    int s, r; //sending and receiving sockets.
    bool _protoTCP;
    //FRAGMENT SIZE IN BYTES FROM BITS ROUNDED UP
    //FSBBU
    unsigned short _fragmentSize;

public:
    //- Constructors -
    /* Ideal para somente envios */
    NetLayer(bool tcp, unsigned short fragment_size/*in bytes*/=MTU);
    /* Ideal para receber numa porta fixa */
    NetLayer(bool tcp, unsigned short local_port, unsigned short fragment_size/*in bytes*/=MTU);
    /* Ideal para spoofar o endereço  de ip local */ //(Se passar)
    NetLayer(bool tcp, unsigned short local_port, char * spoofed_localAddress, unsigned short fragment_size/*in bytes*/=MTU);
    ~NetLayer();
    //- functions ----
    ssize_t SendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    //ssize_t RecvDataFrom(void * buf, size_t len, const char * dest_addr, unsigned short dest_port);
    //Getters&Setters
    struct sockaddr_in GetLocalAddress();
private:
    int _sendNetPackets(net_packet_list * list, const char * dest_addr, unsigned short dest_port);
    /* Initializes upon a random local port by default (local_port=0),
       using spoofed_localAddr=NULL by default means no spoofing Local Address. */
    void _initialize(bool tcp, unsigned short local_port=0, char * spoof_localAddr=NULL, unsigned short fragment_size/*in bytes*/=MTU);
    /* Terminates class allocations */
    void _terminate();
    uint16_t preparePacket(net_packet * p, byte * payload, unsigned short payloadSize, const char * dest_addr, unsigned short dest_port);
}; 
//------------------------------------------------------------------------------
/* 96 bit (12 bytes) pseudo header needed for tcp header checksum calculation */
struct pseudo_header {
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t transport_length;
};

/*	Generic checksum calculation function   */
unsigned short csum(unsigned short *ptr,int nbytes);

