#include <stdio.h>
#include "Serial.h"
#include "TCPClient.h"
#include "TCPServer.h"
#include "Select.h"
char buffer[100];
/*
This example listen to the TCP port using TCPServer and accepts an incoming connection.
After that prints if there is any data sent from the client.
Run this app and from another client or console telnet the particular port  and type something
$telnet 192.168.2.108 9991 

*/
TCPServer TCPs;
TCPClient client;
Select selector;
int len;
void setup()
{
	TCPs.Server("192.168.2.108",9991);        	
	printf("This is setup\n");
}

void loop()
{
	selector.flush();
	selector.add(TCPs.endpoint());
	if(client.endpoint() > 0)
	selector.add(client.endpoint());

	selector.wait();
	printf("After wait\n");

	if(selector.available(TCPs.endpoint()))
	{
		TCPs.Accept(&client);
	}
	if(selector.available(client.endpoint()))
	{
	  len = client.available();
	  client.Read(buffer,len);	
	  for(int i = 0; i < 8; i++)
		printf("{ %c }",buffer[i]);
	  printf("DAta is %d \n",len );

	}
	printf("\nThis is loop %d %d len %d\n",TCPs.endpoint(), client.endpoint(), len);
	
	sleep(1);
}
