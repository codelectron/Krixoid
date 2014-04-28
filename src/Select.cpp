#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include "Select.h"

Select::Select()
{

}

void Select::flush()
{
	FD_ZERO (&fds);
	max_fd = 0;
}
void Select::add(int fd)
{
	FD_SET (fd, &fds);
	if(fd > max_fd)
		max_fd = fd;
}
int Select::wait()
{
	int activity;
	activity = select( max_fd + 1 , &fds , NULL , NULL , NULL);
	if ((activity < 0) && (errno!=EINTR))
        {
           return -1; 
        }
	return activity;

}
int Select::wait(int)
{

}

int  Select::available(int fd)
{
return FD_ISSET(fd, &fds);
}
