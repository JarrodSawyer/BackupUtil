#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "Error.h"


void *clientProcessingThread(void *pClientInformation)
{
  
  printf("Thread created\n");

  sleep(3);
  
  printf("Thread Exiting\n");

  pthread_exit(NULL);
}
