 /* Server program example for IPv4 */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define DEFAULT_PORT 2007
// default TCP socket type
#define DEFAULT_PROTO SOCK_STREAM

void Usage(char *progname)
{
    fprintf(stderr,"Usage: %s -p [protocol] -e [port_num] -i [ip_address]\n", progname);
    fprintf(stderr,"Where:\n\t- protocol is one of TCP or UDP\n");
    fprintf(stderr,"\t- port_num is the port to listen on\n");
    fprintf(stderr,"\t- ip_address is the ip address (in dotted\n");
    fprintf(stderr,"\t  decimal notation) to bind to. But it is not useful here...\n");
    fprintf(stderr,"\t- Hit Ctrl-C to terminate server program...\n");
    fprintf(stderr,"\t- The defaults are TCP, 2007 and INADDR_ANY.\n");
    WSACleanup();
    exit(1);
}

int main(int argc, char **argv)
{
    char Buffer[128];
    char *ip_address= NULL;
    unsigned short port=DEFAULT_PORT;
    int retval;
    int fromlen;
    int i;
    int socket_type = DEFAULT_PROTO;
    struct sockaddr_in local, from;
    WSADATA wsaData;
    SOCKET listen_socket, msgsock;
    
    /* Parse arguments, if there are arguments supplied */
    if (argc > 1){
        for(i=1; i<argc; i++){
            // switches or options...
            if ((argv[i][0] == '-') || (argv[i][0] == '/')){
                // Change to lower...if any
                switch(tolower(argv[i][1])){
                    // if -p or /p
                    case 'p':
                        if (!stricmp(argv[i+1], "TCP"))
                            socket_type = SOCK_STREAM;
                        else if (!stricmp(argv[i+1], "UDP"))
                            socket_type = SOCK_DGRAM;
                        else
                            Usage(argv[0]);
                        i++;
                        break;
                    // if -i or /i, for server it is not so useful...
                    case 'i':
                        ip_address = argv[++i];
                        break;
                    // if -e or /e
                    case 'e':
                        port = atoi(argv[++i]);
                        break;
                    // No match...
                    default:
                        Usage(argv[0]);
                        break;
                    }
            }
            else
                Usage(argv[0]);
        }
    }
    
    // Request Winsock version 2.2
    if ((retval = WSAStartup(0x202, &wsaData)) != 0){
        fprintf(stderr,"Server: WSAStartup() failed with error %d\n", retval);
        WSACleanup();
        return -1;
    }
    else
        printf("Server: WSAStartup() is OK.\n");
    
    if (port == 0)
        Usage(argv[0]);
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = (!ip_address) ? INADDR_ANY:inet_addr(ip_address);
    
    /* Port MUST be in Network Byte Order */
    local.sin_port = htons(port);
    // TCP socket
    listen_socket = socket(AF_INET, socket_type,0);
    
    if (listen_socket == INVALID_SOCKET){
        fprintf(stderr,"Server: socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
        printf("Server: socket() is OK.\n");
    
    // bind() associates a local address and port combination with the socket just created.
    // This is most useful when the application is a
    // server that has a well-known port that clients know about in advance.
    if (bind(listen_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR){
        fprintf(stderr,"Server: bind() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
        printf("Server: bind() is OK.\n");
    
    // So far, everything we did was applicable to TCP as well as UDP.
    // However, there are certain steps that do not work when the server is
    // using UDP. We cannot listen() on a UDP socket.
    if (socket_type != SOCK_DGRAM){
        if (listen(listen_socket,5) == SOCKET_ERROR){
            fprintf(stderr,"Server: listen() failed with error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }
        else
            printf("Server: listen() is OK.\n");
    }
    
    printf("Server: %s: I'm listening and waiting connection\non port %d, protocol %s\n", argv[0], port, (socket_type == SOCK_STREAM)?"TCP":"UDP");
    
    while(1) {
        fromlen =sizeof(from);
        // accept() doesn't make sense on UDP, since we do not listen()
        if (socket_type != SOCK_DGRAM) {
            msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
            if (msgsock == INVALID_SOCKET) {
                fprintf(stderr,"Server: accept() error %d\n", WSAGetLastError());
                WSACleanup();
                return -1;
            }
            else
                printf("Server: accept() is OK.\n");
            printf("Server: accepted connection from %s, port %d\n", inet_ntoa(from.sin_addr), htons(from.sin_port)) ;
        }
        else {
            msgsock = listen_socket;
        }
    
        // In the case of SOCK_STREAM, the server can do recv() and send() on
        // the accepted socket and then close it.
        // However, for SOCK_DGRAM (UDP), the server will do recvfrom() and sendto()  in a loop.
        if (socket_type != SOCK_DGRAM)
            retval = recv(msgsock, Buffer, sizeof(Buffer), 0);
        else{
            retval = recvfrom(msgsock,Buffer, sizeof(Buffer), 0, (struct sockaddr *)&from, &fromlen);
            printf("Server: Received datagram from %s\n", inet_ntoa(from.sin_addr));
        }
        
        
        if (retval == SOCKET_ERROR){
            fprintf(stderr,"Server: recv() failed: error %d\n", WSAGetLastError());
            closesocket(msgsock);
            continue;
        }
        else
            printf("Server: recv() is OK.\n");
        

        if (retval == 0){
            printf("Server: Client closed connection.\n");
            closesocket(msgsock);
            continue;
        }

        printf("Server: Received %d bytes, data \"%s\" from client\n", retval, Buffer);
        printf("Server: Echoing the same data back to client...\n");
        
        if (socket_type != SOCK_DGRAM)
            retval = send(msgsock, Buffer, sizeof(Buffer), 0);
        else
            retval = sendto(msgsock, Buffer, sizeof(Buffer), 0, (struct sockaddr *)&from, fromlen);
    
        if (retval == SOCKET_ERROR)
            fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
        else
            printf("Server: send() is OK.\n");
    
        if (socket_type != SOCK_DGRAM) {
            printf("Server: I'm waiting more connection, try running the client\n");
            printf("Server: program from the same computer or other computer...\n");
            closesocket(msgsock);
        }
        else
            printf("Server: UDP server looping back for more requests\n");
        continue;
    }
    return 0;
}