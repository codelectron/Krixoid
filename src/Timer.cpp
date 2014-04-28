#include "Timer.h"

Timer::Timer()
{

}
void Timer::Set(int time,void (*callback) (int) )
{
  Tvalue.it_interval.tv_sec = time; 
  Tvalue.it_value.tv_sec = 1; 
  StoreTvalue.it_interval.tv_sec = time; 
  StoreTvalue.it_value.tv_sec = 1; 
  CallbackFunc = callback;
  (void) signal(SIGALRM, callback);
  timer_create (CLOCK_REALTIME, NULL, &Timerid);

}
void Timer::Set(int time,int sig, void (*callback) (int) )
{
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  Tvalue.it_interval.tv_sec = time; 
  Tvalue.it_value.tv_sec = 1; 
  StoreTvalue.it_interval.tv_sec = time; 
  StoreTvalue.it_value.tv_sec = 1; 
  CallbackFunc = callback;
  sev.sigev_signo = SIGRTMIN;
  sev.sigev_notify = sig;
  sev.sigev_value.sival_ptr = &Timerid;
  timer_create(CLOCK_REALTIME, &sev, &Timerid);

}


void Timer::NanoSet(int time, int interval)
{

  Tvalue.it_value.tv_nsec = time; 
  Tvalue.it_interval.tv_nsec = interval; 

  StoreTvalue.it_value.tv_nsec = time; 
  StoreTvalue.it_interval.tv_nsec = interval; 
}

void Timer::start()
{
//  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  timer_settime (Timerid, 0, &StoreTvalue, NULL);

}

void Timer::stop()
{
Tvalue.it_value.tv_sec = 0;
Tvalue.it_value.tv_nsec = 0;

Tvalue.it_interval.tv_sec = 0;
Tvalue.it_interval.tv_nsec = 0;
timer_settime (Timerid, 0, &Tvalue, NULL);
}


