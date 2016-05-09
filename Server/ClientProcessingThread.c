#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "Error.h"
#include "ClientMessageHandling.h"

void *clientProcessingThread(void *pClientInformation)
{
  
  debug("Thread created.");

  waitForClientMessages(pClientInformation, NULL);

  sleep(3);
  
  debug("Thread Exiting.");

  pthread_exit(NULL);
}
