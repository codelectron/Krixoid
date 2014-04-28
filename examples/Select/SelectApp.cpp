#include <stdio.h>
#include "Serial.h"
#include "TCPClient.h"
#include "Select.h"
char buffer[100];
/*
This app will open serial device at one end and connects to a TCP server using TCPClient on other end.
The code is tested by printing periodic message from Arduino.
For the server netcat utility was used for testing. The following command invokes a server at the port

$nc -l 5555

*/

TCPClient TCPclient;
Select selector;
Serial serial;
void setup()
{
	serial.Open("/dev/ttyACM0",9600);
	TCPclient.Client("192.168.2.109",5555);        	
	printf("This is setup\n");
}

void loop()
{
 	int len;
	selector.flush();
	selector.add(serial.endpoint());

	selector.wait();

	if(selector.available(serial.endpoint()))
	if(serial.available() )
	{
		len = serial.available();
		serial.Read(buffer, len, 0);
		TCPclient.write((unsigned char *)buffer,len);
		printf("%s\n",buffer);
	}
	printf("This is setup\n");
	sleep(1);
}
