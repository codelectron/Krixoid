#ifndef Twitter_h
#define Twitter_h
#include "tweet.h"
class Twitter
{

public:
	void login(char * ckey, char *csec, char *akey, char *asec);
	void updateStatus(char *message);
	
        union KEYS keys;

	char tckey[100];
	char tcsec[100];
	char takey[100];
	char tasec[100];

};
#endif
