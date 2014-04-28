#ifndef Select_h 
#define Select_h 
#include <sys/select.h>

class Select
{
public:
	Select();
	void flush();
	void add(int);
	int wait();
	int wait(int);
	int  available(int fd);
	fd_set fds;
	int max_fd;
};
#endif
