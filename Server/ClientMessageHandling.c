#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "Error.h"
#include "Messages.h"
#include "ClientMessageHandling.h"

#define MAX_CLIENT_CONNECTIONS 10

typedef struct
{
  int clientFd;
  Messages clientMsg;
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

void * handleNewClientConnection(void *pContext, int clientFd)
{
  ClientMessagingContext *pMessaging = 
    (ClientMessagingContext *) pContext;

  ClientConnectionInformation *pInfo = NULL;

  if(pMessaging != NULL)
  {

    if(pMessaging->numConnectedClients >= MAX_CLIENT_CONNECTIONS)
    {
      ERROR(MAX_CLIENTS_CONNECTED_ERR, "Maximum number of clients reached: %d.", 
	    pMessaging->numConnectedClients);
      return(NULL);
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
	return(NULL);
      }
    }

    pInfo = pMessaging->clientConnections[pMessaging->numConnectedClients];

    memset(pInfo, 0, sizeof(ClientConnectionInformation));
   
    pInfo->clientFd = clientFd;
    
    pMessaging->numConnectedClients++;
    
    return((void *) pInfo);
  }
  
  ERROR(NULL_POINTER, "Context pointer is NULL.");
  return(NULL);
}

int readData(int fd, void *pBuffer, int sizeToRead)
{
  int bytesRead = 0;
  int retVal = 0;

  if(pBuffer == NULL)
  {
    ERROR(NULL_POINTER, "Buffer is NULL");
    return(FAILURE);
  }
  
  if(sizeToRead <=0)
  {
    ERROR(INVALID_BUFFER_SIZE, "Buffer size invalid. Size = %d", sizeToRead);
    return(FAILURE);
  }

  if(fd < 0)
  {
    ERROR(INVALID_SOCKET_ERR, "Socket parameter invalid. Fd = %d", fd);
    return(FAILURE);
  }

  while(bytesRead < sizeToRead)
  {
    retVal = read(fd, pBuffer + bytesRead, sizeToRead - bytesRead); 

    bytesRead += retVal;
  }

  return(bytesRead);
}

int handleTextMessage(void *pContext)
{
  ClientConnectionInformation *pClient = 
    (ClientConnectionInformation *) pContext;
  TextMessage *pMsg = NULL;
  int bytesRead = 0;

  if(pClient != NULL)
  {
    pMsg = &pClient->clientMsg.textMsg;

    bytesRead = readData(pClient->clientFd, &pMsg->text[0], pMsg->hdr.bytesToFollow);
    if(bytesRead != pMsg->hdr.bytesToFollow)
    {
      ERROR(INVALID_MSG_SIZE, "Reading the text message failed. Received: %d Expected: %u", 
	    bytesRead, pMsg->hdr.bytesToFollow);
      return(INVALID_MSG_SIZE);
    }

    pMsg->text[pMsg->hdr.bytesToFollow+1] = '\0';

    debug("Read Text Message: (%s)", pMsg->text);

    return(SUCCESS);
  }
  
  ERROR(NULL_POINTER, "Context pointer is NULL");
  return(NULL_POINTER);
}

int handleClientMessage(void *pContext)
{
  ClientConnectionInformation *pClient = 
    (ClientConnectionInformation *) pContext;
  uint32_t msgLength = 0;
  int bytesRead = 0;
  int retVal = SUCCESS;

  Header *pHdr = &pClient->clientMsg.hdr;
  
  debug("Receiving the header");

  // Read the header
  msgLength = sizeof(pClient->clientMsg.hdr);
  bytesRead = readData(pClient->clientFd, pHdr, msgLength);
  if(bytesRead != msgLength)
  {
    ERROR(INVALID_MSG_SIZE, "Reading the header failed. Received: %d Expected: %u", 
	  bytesRead, msgLength);
    return(INVALID_MSG_SIZE);
  }

  pHdr->msgId = ntohl(pHdr->msgId);
  pHdr->bytesToFollow = ntohl(pHdr->bytesToFollow);
  
  debug("Received HDR. Id: %d Size: %u", pHdr->msgId, pHdr->bytesToFollow);

  // Handle the message
  switch(pHdr->msgId)
  {
    case TEXT_MSG_ID:
      retVal = handleTextMessage(pClient);
      if(retVal != SUCCESS)
      {
	ERROR(retVal, "handleTextMessage() failed");
	return(retVal);
      }
      break;
    default:
      ERROR(INVALID_MSG_ID, "Received invalid message ID: %d", pHdr->msgId);
      return(INVALID_MSG_ID);
  }

  return(SUCCESS);
}

char buffer[256];

int waitForClientMessages(void *pContext, struct timeval *pTimeout)
{
  fd_set readFds;
  int maxFd = -1;
  int retVal = 0;

  ClientConnectionInformation *pClient = 
    (ClientConnectionInformation *) pContext;

  if(pClient != NULL)
  {
    // Clear the fds
    FD_ZERO(&readFds);
    
    debug("The client fd is %d", pClient->clientFd);

    // Add the client fd to set
    FD_SET(pClient->clientFd, &readFds);

    // Max Fd
    maxFd = pClient->clientFd;

    retVal = select(maxFd + 1, &readFds, NULL, NULL, pTimeout);
    if(retVal < 0)
    {
    }
    else if(retVal == 0)
    {
    }
    else
    {
      if(FD_ISSET(pClient->clientFd, &readFds))
      {
	retVal = handleClientMessage(pClient);
	if(retVal != SUCCESS)
	{
	  ERROR(retVal, "Handle client message call returned a failure.");
	}
	else
	{
	  INFO("Message successfully read");
	}
      }
    }

    return(SUCCESS);
  }
  
  return(NULL_POINTER);
}
