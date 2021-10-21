#include "NetLayer.h"
#include <stdio.h>	//for printf
#include <stdlib.h> //for exit(0);
//#include <iostream>
#include <algorithm>
#include <string.h> //for memset
#include <sys/socket.h>	//for socket ofcourse
#include <netdb.h>
#include <arpa/inet.h> // inet_addr

//-- constructors & destructor -----------------------------------------------------------------------------------------
NetLayer::NetLayer(bool tcp, unsigned short fragment_size/*in bytes*/) {
    _initialize(tcp, fragment_size); 
}
NetLayer::NetLayer(bool tcp, unsigned short local_port, unsigned short fragment_size/*in bytes*/) {
    _initialize(tcp, local_port, NULL, fragment_size); 
}
NetLayer::NetLayer(bool tcp, unsigned short local_port, char * spoofed_localAddress, unsigned short fragment_size/*in bytes*/) {
    _initialize(tcp, local_port, spoofed_localAddress, fragment_size);
}
NetLayer::~NetLayer() { _terminate(); }

//-- functions ---------------------------------------------------------------------------------------------------------
ssize_t NetLayer::SendDataTo(const void * buf, size_t len, const char * dest_addr, unsigned short dest_port) { //retornar dados relacionado ao NetLayer (talvez total de pacotes enviados ou codigos de erro)
    //receives raw input to send as a buffer
    //and fills up NetPacket list structure
    net_packet_list packets;
    net_packet * aux;
    int packets_total = (len/_fragmentSize);
    int remainder = (len%_fragmentSize);

    byte * ptr = (byte *) buf;

    if(remainder==0 && packets_total==0) { packets_total=1; }

    int nextPacketOffset=0;//TODO: Re-elaborar o packeting fragments para funcionar corretamente
    for(int i=0;i<packets_total;i++){
        aux = (net_packet *) malloc(sizeof(net_packet));
        preparePacket(aux, ptr, _fragmentSize, dest_addr, dest_port);
        ptr=(byte*)buf+_fragmentSize;
        packets.push_back(*aux);
    }    

    if(remainder != 0){
        aux = (net_packet *) malloc(sizeof(net_packet));
        preparePacket(aux, ptr, remainder, dest_addr, dest_port);
        packets.push_back(*aux);
    }

    //call f( _sendNetPackets ) to send netpackets
    _sendNetPackets(&packets, dest_addr, dest_port);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
uint16_t NetLayer::preparePacket(net_packet * p, byte * payload, unsigned short payloadSize, const char * dest_addr, unsigned short dest_port) { /*return next packet offset*/
    uint16_t ret=0;
    memset(p->datagram, 0, MTU);
    byte * data;
    p->iph = (struct iphdr *)(p->datagram);
    if(_protoTCP){
        p->tcph = (struct tcphdr *) ((p->datagram) + sizeof(struct ip));
        p->udph = NULL;
        data = p->datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
    }
    else{
        p->udph = (struct udphdr *) ((p->datagram) + sizeof(struct ip));
        p->tcph = NULL;
        data = p->datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    }
    
    memcpy(data, payload, payloadSize);

    p->iph->version = 4;
    p->iph->ihl = 5;
    p->iph->tos = 0;
    p->iph->tot_len = sizeof(struct iphdr) + ((_protoTCP)?sizeof(struct tcphdr):sizeof(struct udphdr)) + payloadSize;
    p->iph->id = htons(54321);//posso deixar fixo mas seria legal variar para fragmentacao e reassembly                 *
//...
    p->iph->frag_off = 0;
//...
    p->iph->ttl = 255;
    p->iph->protocol = (_protoTCP) ? IPPROTO_TCP : IPPROTO_UDP;
    p->iph->check = 0; //seta ser zero antes de fazer o calculo
    p->iph->saddr = localaddr.sin_addr.s_addr;
    p->iph->daddr = inet_addr(dest_addr);
    //ip checksum
    p->iph->check = csum ((unsigned short*)p->datagram, p->iph->tot_len);
    if(_protoTCP){
        p->tcph->source = localaddr.sin_port;
        p->tcph->dest = htons(dest_port);
        p->tcph->seq = 0;
        p->tcph->ack_seq = 0;
        p->tcph->doff = 5;	//tcp header size
        p->tcph->fin = 0;
        p->tcph->syn = 1;
        p->tcph->rst = 0;
        p->tcph->psh = 0;
        p->tcph->ack = 0;
        p->tcph->urg = 0;
        p->tcph->window = htons (5840);	/* maximum allowed window size */
        p->tcph->check = 0;	//leave checksum 0 now, filled later by pseudo header
        p->tcph->urg_ptr = 0;
    } else {
        p->udph->source = localaddr.sin_port;
        p->udph->dest = htons(dest_port);
        int aux = sizeof(struct udphdr) + payloadSize;
        //aux += (aux%8==0)?0:1;
        p->udph->len = htons(aux);
        p->udph->check = 0;
    }

    struct pseudo_header psh;
    psh.source_address = p->iph->saddr;
	psh.dest_address = p->iph->daddr;
	psh.placeholder = 0;
	psh.protocol = (_protoTCP) ? IPPROTO_TCP : IPPROTO_UDP;
    psh.transport_length = htons(((_protoTCP) ? sizeof(struct tcphdr) : sizeof(struct udphdr)) + payloadSize);
	
	int psize = sizeof(struct pseudo_header) + ((_protoTCP)? sizeof(struct tcphdr):sizeof(struct udphdr)) + payloadSize; 
	
    char * pseudogram = (char*) malloc(psize);
	memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
	
    if(_protoTCP){
        memcpy(pseudogram + sizeof(struct pseudo_header) , &p->tcph , (sizeof(struct tcphdr) + payloadSize));
        p->tcph->check = csum((unsigned short*) pseudogram , psize);
    }
	    
    else {
        memcpy(pseudogram + sizeof(struct pseudo_header) , &p->udph , (sizeof(struct udphdr) + payloadSize));
        p->udph->check = csum((unsigned short*) pseudogram , psize);
    }
        
	free(pseudogram);
    return ret;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
int NetLayer::_sendNetPackets(net_packet_list * list, const char * dest_addr, unsigned short dest_port) {
    struct sockaddr_in dest;
    dest.sin_addr.s_addr = inet_addr(dest_addr);
    dest.sin_family = AF_INET;
    dest.sin_port = dest_port;
    int count=0;
    for(net_packet_list::iterator it=list->begin();it!=list->end();++it){
        if (sendto(s, it->datagram, (size_t)it->iph->tot_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("sendto failed");
            return -1;
        }
        else
            printf ("(%d) Packet Send. Length : %d \n" , count++, it->iph->tot_len);
        sleep(1);
    }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void NetLayer::_initialize(bool tcp, unsigned short local_port, char * spoof_localAddr, unsigned short fragment_size/*in bytes*/) {
    _fragmentSize = (fragment_size>MTU) ? MTU : fragment_size; 
    _protoTCP = tcp;
    s = socket(PF_INET, SOCK_RAW, ((tcp) ? IPPROTO_TCP : IPPROTO_UDP));
    if (s==-1) {
        perror("Failed to create socket");
        return;
    }

    if(local_port==0){
        sockaddr_in tmp;
        socklen_t len = sizeof(tmp);
        if (getsockname(s, (struct sockaddr *)&tmp, &len) < 0){ //!! por que ele atribui a porta 17?
            perror("failed to get hostname with errno: ");
            return;
        }
        localaddr.sin_port = tmp.sin_port;
    }
    else
        localaddr.sin_port = htons(local_port);

    localaddr.sin_family = AF_INET;
    
    if(spoof_localAddr != NULL)
        localaddr.sin_addr.s_addr = inet_addr(spoof_localAddr);
    else {
        //get and set local address from computer
        char tmp[256];
        if(gethostname(tmp, sizeof(tmp)) == -1){
            perror("gethostname() ");
            return;
        }
        struct hostent * host_entry = gethostbyname(tmp);
        if(host_entry == NULL) {
            perror("gethostname() ");
            return;
        }
        char * ip = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
        localaddr.sin_addr.s_addr = inet_addr(ip);
    }

    //IP_HDRINCL to tell the kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;
	
	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
		perror("Error setting IP_HDRINCL");
		return;
	}

}
void NetLayer::_terminate() {
    //destroy package list
}

struct sockaddr_in NetLayer::GetLocalAddress(){
    struct sockaddr_in ret = {
        .sin_port = localaddr.sin_port,
        .sin_addr = localaddr.sin_addr
    };
    ret.sin_family = localaddr.sin_family;
    return ret;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
/*	Generic checksum calculation function   */
unsigned short csum(unsigned short *ptr,int nbytes)  {
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}
