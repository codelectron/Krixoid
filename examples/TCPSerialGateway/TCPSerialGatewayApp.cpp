#include <stdio.h>
#include "Serial.h"
#include "TCPClient.h"
#include "Krixoid.h"
char buffer[100];
/*
This app will open serial device at one end and connects to a TCP server using TCPClient on other end.
The code is tested by printing periodic message from Arduino.
For the server netcat utility was used for testing. The following command invokes a server at the port

$nc -l 5555

*/
TCPClient TCPclient;
Serial serial;
void setup()
{
	serial.Open("/dev/ttyACM0",9600);
	TCPclient.Client("192.168.2.109",5555);        	
	printf("This is setup\n");
}

void loop()
{

	if(serial.available() > 8)
	{
		serial.Read(buffer, 8, 0);
		TCPclient.write((unsigned char *)buffer,8);
		printf("%s\n",buffer);
	}
	printf("This is setup\n");
	Sleep(1);
}
