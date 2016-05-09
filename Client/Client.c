#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include <string.h>

#include "Error.h"
#include "Debug.h"
#include "Messages.h"

int main(int argc, char *argv[])
{
  int fd = 0;
  struct sockaddr_in serv_addr; 
  TextMessage msg;
  
  memset(&msg, 0,sizeof(msg));
  if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Error : Could not create socket \n");
    return 1;
  } 

  memset(&serv_addr, '0', sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(5000); 

  if(inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr)<=0)
  {
    printf("\n inet_pton error occured\n");
    return 1;
  } 

  if( connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\n Error : Connect Failed \n");
    return 1;
  } 

  sleep(5);

  msg.hdr.msgId = htonl(TEXT_MSG_ID);
  msg.hdr.bytesToFollow = htonl(strlen("Test Message") - sizeof(Header));
  
  strcpy(&msg.text[0], "Test Message");

  write(fd, &msg.hdr, sizeof(Header));
  write(fd, &msg.text[0], ntohl(msg.hdr.bytesToFollow));
  
  shutdown(fd,SHUT_RDWR);
  close(fd);

  return(SUCCESS);
}
