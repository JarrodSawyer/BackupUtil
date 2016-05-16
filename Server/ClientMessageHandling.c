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
  int* pClientFd;
  Messages clientMsg;
} ClientConnectionInformation;

int cleanupClientMessaging(void *pContext)
{
  if(pContext != NULL)
  {
    ClientConnectionInformation *pInfo =
      (ClientConnectionInformation *) pContext;
    
    if(pInfo->pClientFd != NULL)
    {
      if(*pInfo->pClientFd >= 0)
      {
	close(*pInfo->pClientFd);
      }

      free(pInfo->pClientFd);
    }

    free(pContext);
    pContext = NULL;

    return(SUCCESS);
  }

  ERROR(NULL_POINTER, "Cleanup function receoved null context pointer");
  return(NULL_POINTER);
}

void * handleNewClientConnection(int *pClientFd)
{
  ClientConnectionInformation *pInfo = NULL;

  pInfo =
    (ClientConnectionInformation *) malloc(sizeof(ClientConnectionInformation));
 
  if(pInfo == NULL)
  {
    ERROR(ALLOCATION_ERR, "Failed to allocate client connection information structure. Client FD %d", *pClientFd);
    return(NULL);
  }
  
  memset(pInfo, 0, sizeof(ClientConnectionInformation));
   
  pInfo->pClientFd = pClientFd;
  
  return((void *) pInfo);
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

    bytesRead = readData(*pClient->pClientFd, &pMsg->text[0], pMsg->hdr.bytesToFollow);
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
  bytesRead = readData(*pClient->pClientFd, pHdr, msgLength);
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
    
    debug("The client fd is %d", *pClient->pClientFd);

    // Add the client fd to set
    FD_SET(*pClient->pClientFd, &readFds);

    // Max Fd
    maxFd = *pClient->pClientFd;

    retVal = select(maxFd + 1, &readFds, NULL, NULL, pTimeout);
    if(retVal < 0)
    {
    }
    else if(retVal == 0)
    {
    }
    else
    {
      if(FD_ISSET(*pClient->pClientFd, &readFds))
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
