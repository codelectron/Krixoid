#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/ioctl.h>
#include "TCPClient.h"

TCPClient::TCPClient()
{

}

int TCPClient::write(uint8_t *buffer, size_t size)
{
        return send(new_socket, buffer, size, 0);
}

int TCPClient::Read(void)
{

}
int TCPClient::Read( char * buffer, int length)
{
 return read( new_socket , buffer, length);
}

int TCPClient::Client(char * ipaddress, int port){
   int ret;
   int counter = 0;

    this->ipaddress = ipaddress;
    this->port = port;
    if ((new_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        // Handle Error
    }
   memset(&ClientAddr, 0, sizeof(ClientAddr));   /* Zero out structure */
   ClientAddr.sin_family = AF_INET;
   ClientAddr.sin_addr.s_addr=inet_addr(ipaddress);
   ClientAddr.sin_port=htons(port);
   sleep(1);
   do
   {
       ret = connect(new_socket , (struct sockaddr *)&ClientAddr, sizeof(ClientAddr));
       if( ret < 0)
       {
         printf("Error connecting to server\n");
        counter++;
        sleep(counter);
        if(counter == 20)
                exit(1);
        }

   }while(ret < 0);

}

int TCPClient::endpoint()
{
  return new_socket;
}
int TCPClient::available(void)
{
        int count;
        ioctl(sock_fd, FIONREAD, &count);
        return count;
}

