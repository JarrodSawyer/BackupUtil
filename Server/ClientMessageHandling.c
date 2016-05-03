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

  ERROR(NULL_POINTER, "Cleanup function receoved null context pointer");
  return(NULL_POINTER);
}

void* initializeClientMessaging()
{
  ClientMessagingContext *pMessagingContext = NULL;
  int idx = 0;

  pMessagingContext = (ClientMessagingContext *) malloc(sizeof(ClientMessagingContext));
  if(pMessagingContext == NULL)
  {
    ERROR(ALLOCATION_ERR, "Failed to allocate the Client Messaging Context.");
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

  if(pMessaging != NULL)
  {

    if(pMessaging->numConnectedClients >= MAX_CLIENT_CONNECTIONS)
    {
      ERROR(MAX_CLIENTS_CONNECTED_ERR, "Maximum number of clients reached: %d.", 
	    pMessaging->numConnectedClients);
      return(MAX_CLIENTS_CONNECTED_ERR);
    }

    // Allocate client connection structure
    if(pMessaging->clientConnections[pMessaging->numConnectedClients] == NULL)
    {

      pMessaging->clientConnections[pMessaging->numConnectedClients] =
	(ClientConnectionInformation *) malloc(sizeof(ClientConnectionInformation));
 
      if(pMessaging->clientConnections[pMessaging->numConnectedClients] == NULL)
      {
	ERROR(ALLOCATION_ERR, "Failed to allocate client connection information structure. Client number %d", 
	      pMessaging->numConnectedClients);
	return(ALLOCATION_ERR);
      }
    }
   
    pMessaging->clientConnections[pMessaging->numConnectedClients]->clientFd = clientFd;

    pMessaging->numConnectedClients++;
    
    return(SUCCESS);
  }
  
  ERROR(NULL_POINTER, "Context pointer is NULL.");
  return(NULL_POINTER);
}
