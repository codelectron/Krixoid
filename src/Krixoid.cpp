#include "Krixoid.h"
#include <time.h>

void Sleep(time_t time)
{
   struct timespec tim;
   tim.tv_sec = time;
   tim.tv_nsec = 0;
   nanosleep(&tim , NULL);
}
