#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <string.h>

#include "Error.h"
#include "Debug.h"
#include "Messages.h"

#define OPTIONS_STR "p:i:"
static struct option LongOptions[] = 
{
  {"port", required_argument, 0, 'p'},
  {"ip",   required_argument, 0, 'i'},
  {0,      0,                 0, 0  }
};


#define DEFAULT_SERVER_PORT 5001
#define DEFAULT_SERVER_IP "0.0.0.0"

#define IP_MAX_LEN 16
typedef struct
{
  uint16_t serverPortNum;
  char serverIpStr[IP_MAX_LEN + 1];

  int connectionFd;
} ClientInformation;

void printUsage()
{
  printf("Usage: ./Client -i SERVER_IP_ADDRESS -p PORTNUM\n");
}

int parseArguments(int argc, char *argv[], ClientInformation *pClientInfo)
{
  int opt = 0;
  int longIdx = 0;

  while((opt = getopt_long(argc, argv, OPTIONS_STR, LongOptions, &longIdx)) != -1)
  {
    switch(opt)
    {
      case 'p':
	pClientInfo->serverPortNum = atoi(optarg);
	break;
      case 'i':
	strncpy(&pClientInfo->serverIpStr[0], optarg, IP_MAX_LEN);
	pClientInfo->serverIpStr[IP_MAX_LEN] = '\0';
	break;
      default:
	printUsage();
	exit(1);
	break;
    }
  }

  return(SUCCESS);
}

int initializeClientInformation(ClientInformation *pClientInfo)
{
  if(pClientInfo != NULL)
  {
    pClientInfo->serverPortNum = DEFAULT_SERVER_PORT;

    memset(&pClientInfo->serverIpStr[0], 0, sizeof(pClientInfo->serverIpStr));
    strncpy(&pClientInfo->serverIpStr[0], DEFAULT_SERVER_IP, strlen(DEFAULT_SERVER_IP));

    pClientInfo->connectionFd = -1;

    return(SUCCESS);
  }

  ERROR(NULL_POINTER, "Function received NULL ClientInformation pointer.");
  return(NULL_POINTER);
}

int connectToServer(ClientInformation *pClientInfo)
{
  struct sockaddr_in serv_addr; 
  int retVal = SUCCESS;

  if(pClientInfo != NULL)
  {
    pClientInfo->connectionFd = socket(AF_INET, SOCK_STREAM, 0);
    if(pClientInfo->connectionFd < 0)
    {
      ERROR(SOCKET_CREATION_ERR, "Failed to create socket.");
      return(SOCKET_CREATION_ERR);
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(pClientInfo->serverPortNum); 

    retVal = inet_pton(AF_INET, pClientInfo->serverIpStr, &serv_addr.sin_addr);
    if(retVal <= 0)
    {
      ERROR(FAILURE, "inet_pton call failed");
      return(FAILURE);
    } 

    retVal = connect(pClientInfo->connectionFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(retVal  < 0)
    {
      ERROR(SOCKET_CONNECT_ERR, "Conenct call failed");
      return(SOCKET_CONNECT_ERR);
    } 

    return(SUCCESS);
  }

  ERROR(NULL_POINTER, "Function received NULL ClientInformation");
  
  return(NULL_POINTER);
}

int main(int argc, char *argv[])
{
  ClientInformation clientInfo;
  TextMessage msg;
  int retVal = SUCCESS;
  
  retVal = initializeClientInformation(&clientInfo);
  if(retVal != SUCCESS)
  {
    ERROR(retVal, "Error initializing client information.");
    return(FAILURE);
  }

  retVal = parseArguments(argc, argv, &clientInfo);
  if(retVal != SUCCESS)
  {
    ERROR(retVal, "Error parsing arguments");
    return(FAILURE);
  }

  debug("Server Port: %d Ip: %s", clientInfo.serverPortNum, clientInfo.serverIpStr);

  retVal = connectToServer(&clientInfo);
  if(retVal != SUCCESS)
  {
    ERROR(retVal, "connectToServer() failed");
    return(FAILURE);
  }

  memset(&msg, 0,sizeof(msg));
  
  sleep(5);

  msg.hdr.msgId = htonl(TEXT_MSG_ID);
  msg.hdr.bytesToFollow = htonl(strlen("Test Message") - sizeof(Header));
  
  strcpy(&msg.text[0], "Test Message");

  write(clientInfo.connectionFd, &msg.hdr, sizeof(Header));
  write(clientInfo.connectionFd, &msg.text[0], ntohl(msg.hdr.bytesToFollow));
  
  shutdown(clientInfo.connectionFd,SHUT_RDWR);
  close(clientInfo.connectionFd);

  return(SUCCESS);
}
