#include <stdio.h>
#include <unistd.h>
#include "Timer.h"
#include "Krixoid.h"

Timer timer;
TimerFunc Calleb;
TimerFunc Callebb;

void Calleb1(int cal);
void setup()
{
 	Calleb = Calleb1; 
	timer.Set(2,Calleb);
	timer.start();
	Sleep(100);
}

void loop()
{
	printf("This is here \n");
	timer.stop();
}

void Calleb1(int cal)
{
 printf("I am called  back 1  %d\n", cal);
}
