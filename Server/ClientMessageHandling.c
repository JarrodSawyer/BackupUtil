#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Error.h"
#include "ClientMessageHandling.h"

#define MAX_CLIENT_CONNECTIONS 10

typedef struct
{
  int clientFd;
} ClientConnectionInformation;

typedef struct
{
  int numConnectedClients;
  ClientConnectionInformation *clientConnections[MAX_CLIENT_CONNECTIONS];
} ClientMessagingContext;

int cleanupClientMessaging(void *pContext)
{
  int idx = 0;
  
  if(pContext != NULL)
  {
    ClientMessagingContext *pMessaging =
      (ClientMessagingContext *) pContext;

    for(idx = 0; idx < pMessaging->numConnectedClients; idx++)
    {
      if(pMessaging->clientConnections[idx] != NULL)
      {
	if(pMessaging->clientConnections[idx]->clientFd >=0)
	{
	  close(pMessaging->clientConnections[idx]->clientFd);
	}

	free(pMessaging->clientConnections[idx]);
      }
    }

    free(pContext);

    return(SUCCESS);
  }

  return(NULL_POINTER);
}

void* initializeClientMessaging()
{
  ClientMessagingContext *pMessagingContext = NULL;
  int idx = 0;

  pMessagingContext = (ClientMessagingContext *) malloc(sizeof(ClientMessagingContext));
  if(pMessagingContext == NULL)
  {
    return(NULL);
  }
    
  pMessagingContext->numConnectedClients = 0;
  
  for(idx = 0; idx < MAX_CLIENT_CONNECTIONS; idx++)
  {
    pMessagingContext->clientConnections[idx] = NULL;
  }

  return((void *) pMessagingContext);
}

int handleNewClientConnection(void *pContext, int clientFd)
{
  ClientMessagingContext *pMessaging = 
    (ClientMessagingContext *) pContext;
  printf("Line: %d\n", __LINE__);
  if(pMessaging != NULL)
  {
printf("Line: %d\n", __LINE__);
    if(pMessaging->numConnectedClients >= MAX_CLIENT_CONNECTIONS)
    {
      return(MAX_CLIENTS_CONNECTED_ERR);
    }
    printf("Line: %d %d\n", __LINE__, pMessaging->numConnectedClients);
    // Allocate client connection structure
    if(pMessaging->clientConnections[pMessaging->numConnectedClients] == NULL)
    {
printf("Line: %d\n", __LINE__);
      pMessaging->clientConnections[pMessaging->numConnectedClients] =
	(ClientConnectionInformation *) malloc(sizeof(ClientConnectionInformation));
 printf("Line: %d\n", __LINE__);
 
      if(pMessaging->clientConnections[pMessaging->numConnectedClients] == NULL)
      {
	return(ALLOCATION_ERR);
      }
    }
 printf("Line: %d\n", __LINE__);
  
    pMessaging->clientConnections[pMessaging->numConnectedClients]->clientFd = clientFd;

    pMessaging->numConnectedClients++;
    
    return(SUCCESS);
  }
  
  return(NULL_POINTER);
}
