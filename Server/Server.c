#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>

#include <string.h>

#include "Error.h"
#include "ClientMessageHandling.h"
#include "ClientProcessingThread.h"

#define DEFAULT_SERVER_LISTEN_PORT 5001
#define SERVER_LISTEN_BACKLOG 10

#define MAX_NUM_THREADS 50

#define OPTIONS_STR "p:"//i:"
static struct option LongOptions[] = 
{
  {"port", required_argument, 0, 'p'},
  //{"ip",   required_argument, 0, 'i'},
  {0,      0,                 0, 0  }
};

typedef struct
{
  uint16_t portNum;
  int serverSocket;
  fd_set readFds;
  int maxFd;

  //Thread Info
  int numThreads;
  pthread_t threadIds[MAX_NUM_THREADS];

  void *pClientMessaging;
} ServerInformation;

int initializeServerInformation(ServerInformation *pServerInfo)
{
  int retVal = SUCCESS;

  if(pServerInfo != NULL)
  {
    pServerInfo->pClientMessaging = NULL;

    pServerInfo->pClientMessaging = initializeClientMessaging();
    if(pServerInfo->pClientMessaging == NULL)
    {
      return(ALLOCATION_ERR);
    }
    
    pServerInfo->serverSocket = -1;
    pServerInfo->maxFd = -1;
    FD_ZERO(&pServerInfo->readFds);

    pServerInfo->numThreads = 0;
    pServerInfo->portNum = DEFAULT_SERVER_LISTEN_PORT;

    return(SUCCESS);
  }
  
  return(NULL_POINTER);
}

int initializeServerSocket(ServerInformation *pServerInfo)
{
  struct sockaddr_in serv_addr;

  if(pServerInfo != NULL)
  {
    // Create the server socket
    pServerInfo->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pServerInfo->serverSocket < 0) 
    {
      return(SOCKET_CREATION_ERR);
    }

    // Initialize socket structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
        
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(pServerInfo->portNum);
  
    // Now bind the host address
    if (bind(pServerInfo->serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
      return(SOCKET_BIND_ERR);
    }
    
    if(listen(pServerInfo->serverSocket, SERVER_LISTEN_BACKLOG) < 0)
    {
      return(SOCKET_LISTEN_ERR);
    }

    return(SUCCESS);
  }

  return(NULL_POINTER);
}

int serverHandleClientConnection(ServerInformation *pServerInfo)
{
  struct sockaddr_in cli_addr;
  unsigned int clilen = 0;
  int newSockFd = -1;
  int retVal = SUCCESS;
  printf("Line: %d\n", __LINE__);
  if(pServerInfo != NULL)
  {
    clilen = sizeof(cli_addr);
printf("Line: %d\n", __LINE__);
    newSockFd = accept(pServerInfo->serverSocket, (struct sockaddr *) &cli_addr, &clilen);
    if (newSockFd < 0) 
    {
      return(SOCKET_ACCEPT_ERR);
    }
printf("Line: %d\n", __LINE__);
    retVal = handleNewClientConnection(pServerInfo->pClientMessaging, newSockFd);
    if(retVal != SUCCESS)
    {
      return(retVal);
    }
printf("Line: %d\n", __LINE__);
    /* Create client connection thread */
    retVal = pthread_create(&pServerInfo->threadIds[pServerInfo->numThreads], NULL, &clientProcessingThread, NULL);
    if(retVal != 0)
    {
      return(THREAD_CREATION_ERR);
    }
printf("Line: %d\n", __LINE__);
    pServerInfo->numThreads++;
printf("Line: %d\n", __LINE__);
    return(SUCCESS);
  }

  return(NULL_POINTER);
}

int serverCleanup(ServerInformation *pServerInfo)
{
  if(pServerInfo != NULL)
  {
    if(pServerInfo->serverSocket >= 0)
    {
      close(pServerInfo->serverSocket);
    }

    cleanupClientMessaging(pServerInfo->pClientMessaging);

    return(SUCCESS);
  }

  return(NULL_POINTER);
}

void printUsage()
{
  printf("Usage: ./Server -i IP_ADDRESS -p PORTNUM\n");
}

int parseArguments(int argc, char *argv[], ServerInformation *pServerInfo)
{
  int opt = 0;
  int longIdx = 0;

  while((opt = getopt_long(argc, argv, OPTIONS_STR, LongOptions, &longIdx)) != -1)
  {
    switch(opt)
    {
      case 'p':
	pServerInfo->portNum = atoi(optarg);
	break;
	//case 'i':
	//break;
      default:
	printUsage();
	exit(1);
	break;
    }
  }

  return(SUCCESS);
}

/**
 * Socket code taken from http://www.tutorialspoint.com/unix_sockets/socket_server_example.htm. 
 **/

int main(int argc, char *argv[]) 
{
  ServerInformation serverInfo;
  int retVal = SUCCESS;

  // Initialize Server Information
  retVal = initializeServerInformation(&serverInfo);
  if(retVal != SUCCESS)
  {
    fprintf(stderr, "Error initializing server information. Error code: %d\n", retVal);
    exit(1);
  }

  retVal = parseArguments(argc, argv, &serverInfo);
  if(retVal != SUCCESS)
  {
    fprintf(stderr, "Error parsing arguments. Error Code: %d\n", retVal);
    printUsage();
    exit(1);
  }

  retVal = initializeServerSocket(&serverInfo);
  if(retVal != SUCCESS)
  {
    fprintf(stderr, "Error initializing server socket. ErrorCode: %d\n", retVal);
    serverCleanup(&serverInfo);
    exit(1);
  }

  while (1) {

    // Clear the fds
    FD_ZERO(&serverInfo.readFds);

    // Add server fd to the set
    FD_SET(serverInfo.serverSocket, &serverInfo.readFds);

    //Set max fd
    serverInfo.maxFd = serverInfo.serverSocket;

    retVal = select(serverInfo.maxFd+1, &serverInfo.readFds, NULL, NULL, NULL);
    if(retVal < 0)
    {
      fprintf(stderr, "Error on select. Errno: %d\n", errno);
    }
    else if(retVal == 0) // Select timeout. Nothing to read 
    {
      printf("Select timeout\n");
    }
    else
    {
      if(FD_ISSET(serverInfo.serverSocket, &serverInfo.readFds))
      {
	retVal = serverHandleClientConnection(&serverInfo);
	if(retVal != SUCCESS)
	{
	  fprintf(stderr, "Error handling client connection. Error Code: %d\n", retVal);
	}
      }
    }  
       
  } /* end of while */
}
