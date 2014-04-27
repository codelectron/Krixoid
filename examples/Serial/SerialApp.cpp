/*
Author: Krishna Pattabiraman
Date: 27/04/2014
Contact:www.codelectron.com
This example opens the serial interface ttyACM0 at baudrate 9600 and prints out when there is any data available.
This was tested with Arduino connected to Raspberry pi printing some message periodically
*/

#include <stdio.h>
#include <Serial.h>
char buffer[100];
Serial serial;
void setup()
{
  int ret;
  ret = serial.Open("/dev/ttyACM0",9600);
  if(ret < 0)
  { 
	printf("Unable to open the serial device\n");
	exit(1);

  }
  
}


void loop()
{
	int len;
	if(serial.available())
	{
		len = serial.available();
	
		serial.Read(buffer, len, 0);

		printf("%s\n",buffer);
	}
	printf("This is setup\n");
	sleep(1);

}
