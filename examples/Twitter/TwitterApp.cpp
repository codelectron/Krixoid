#include <stdio.h>
#include <unistd.h>
extern "C" {
#include <oauth.h>
 }
#include "Twitter.h"
char buffer[100];
/*
This app will send a status update to the twitter account. For doing that
you need to get twitter developer login and get the credentials 
such as consumer key ans secret and authentication key and secret.

This app is developed keeping usefulness and automation in mind, users need to 
respect and follow  "Developer Rules of the Road" https://dev.twitter.com/terms/api-terms
*/
char c_key[] = "xxxxxxxxxxxxxxxxxxxx";
char c_sec[] = "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
char t_key[]  = "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
char  t_sec[]  = "*********************************************";

Twitter twitter;
void setup()
{
  twitter.login(c_key, c_sec, t_key, t_sec);
}

void loop()
{
	printf("Here\n");
        twitter.updateStatus("code");	
	printf("after\n");
	sleep(10);
}
