#ifndef Pthread_h
#define Pthread_h
#include <pthread.h>
#include <sys/signal.h>
class PThread
{

public:
	PThread();
	void create(void *(*start_routine)(void*));
	void start();
	int getStatus();
	void wait();
	void stop();
	pthread_t thread;
	void *(*thread_routine) (void *);


};


#endif
