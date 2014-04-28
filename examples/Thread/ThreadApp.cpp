#include "PThread.h"
#include "Krixoid.h"
#include <stdio.h>


void* print_thread(void*);
PThread th;

void setup()
{
	th.create(print_thread);
	th.start();
}

void loop()
{
	Sleep(10);
	th.stop();
}

void* print_thread(void*) {
	while(1)
	fprintf(stdout,"Testing Thread execution and exit\n");
}
