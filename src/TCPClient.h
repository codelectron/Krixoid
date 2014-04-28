#ifndef TCPClient_h
#define TCPClient_h 

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

class TCPClient{
public:
        TCPClient();
        int Client(char * ipaddress, int port);
        int write(uint8_t);
        int write(uint8_t *buffer, size_t size);
        int Read(void);
        int Read( char * buffer, int length);
        int available(void);
	int endpoint();

        int type;
        char * ipaddress;
        int port;
        int sock_fd;
        int device_fd;
        fd_set readfds;
        struct sockaddr_in ClientAddr; /* Local address */
        int new_socket , client_socket[30];

};

#endif
