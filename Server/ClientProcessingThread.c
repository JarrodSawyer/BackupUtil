#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "Error.h"
#include "ClientMessageHandling.h"

void *clientProcessingThread(void *pClientConnectionFd)
{
  void *pClientConnectionInfo = NULL;
  
  debug("Thread created.");

  if(pClientConnectionFd == NULL)
  {
    ERROR(NULL_POINTER, "Client Processing Thread argument is NULL");
    pthread_exit(NULL);
  }

  pClientConnectionInfo = handleNewClientConnection(pClientConnectionFd);
  if(pClientConnectionInfo == NULL)
  {
    ERROR(ALLOCATION_ERR, "handleNewClientConnection call failed. Client Info Pointer: %p", pClientConnectionInfo);
    free(pClientConnectionFd);
    pthread_exit(NULL);
  }

  waitForClientMessages(pClientConnectionInfo, NULL);

  sleep(3);
  
  debug("Thread Exiting.");

  cleanupClientMessaging(pClientConnectionInfo);

  pthread_exit(NULL);
}
