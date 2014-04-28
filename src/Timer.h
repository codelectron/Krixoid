#ifndef Timer_h
#define Timer_h

#include <stdio.h>
#include <time.h>
#include <signal.h>

typedef void (*TimerFunc)(int);
class Timer
{
	public:
		Timer();
		void Set(int, void (*callback) (int));
		void Set(int time,int sig, void (*callback) (int) );
		void NanoSet(int, int);
		void start();
		void stop();
		timer_t Timerid;
		struct itimerspec Tvalue;
		struct itimerspec StoreTvalue;
		void (*CallbackFunc)(int);
	        struct sigevent sev;
		sigset_t mask;
};
#endif
