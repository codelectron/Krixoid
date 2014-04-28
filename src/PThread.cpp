#include "PThread.h"
#include <pthread.h>
#include <signal.h>
#include <sys/signal.h>

PThread::PThread()
{

}


void PThread::create(void *(*start_routine)(void*))
{
    thread_routine = start_routine;
}

void PThread::start()
{
     pthread_create(&thread, NULL, thread_routine, NULL);
}

int PThread::getStatus()
{

}
 
void PThread::wait()
{
    pthread_join(thread,NULL);
}

void PThread::stop()
{
  pthread_cancel(thread);
}
