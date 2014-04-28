#include "TCPServer.h"
#include "TCPClient.h"


TCPServer::TCPServer()
{

}

int TCPServer::Server(char * ipaddress, int port){
	this->ipaddress = ipaddress;
	this->port = port;

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		// Handle Error
	}
	/* Construct local address structure */
	memset(&ServAddr, 0, sizeof(ServAddr));   /* Zero out structure */
	ServAddr.sin_family = AF_INET;                /* Internet address family */
	//    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	inet_pton(AF_INET, ipaddress, &ServAddr.sin_addr.s_addr);
	ServAddr.sin_port = htons(port);      /* Local port */

	/* Bind to the local address */
	if (bind(sock_fd, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
	{
		printf("\nBind Error\n");
		// Handle Error
	}    /* Mark the socket so it will listen for incoming connections */
	if (listen(sock_fd, 5) < 0)
	{
		printf("\nListen Error\n");
		// Handle Error
	}
}

int TCPServer::Accept(TCPClient * client)
{
	int clientlen;
	struct sockaddr * remoteAddr = ( struct sockaddr *)&client->ClientAddr;
	clientlen = sizeof(client->ClientAddr);
	if ((client->new_socket = accept(sock_fd, (struct sockaddr *)remoteAddr, (socklen_t*)&clientlen))<0)
	{
		return -1;
	}
	else
	{
		return client->new_socket;
	}
}

int TCPServer::endpoint()
{
   return sock_fd;
}
