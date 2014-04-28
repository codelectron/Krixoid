#include "Twitter.h"
#include "tweet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <oauth.h>
 }
#include <curl/curl.h>

void Twitter::login(char * ckey, char *csec, char *akey, char *asec)
{
  char *rep = NULL;

  strcpy(tckey, ckey);
  strcpy(tcsec, csec);
  strcpy(takey, akey);
  strcpy(tasec, asec);
  
  keys.keys_array[0] = tckey; 
  keys.keys_array[1] = tcsec; 
  keys.keys_array[2] = takey; 
  keys.keys_array[3] = tasec;

  bear_init(&keys);
}

void Twitter::updateStatus(char *message)
{
    char * rep = NULL;
    post_statuses_update(message, &rep, 0, 0, (struct GEOCODE){0, 0, 0, ""}, 0, -1, -1);
    free(rep);
}


