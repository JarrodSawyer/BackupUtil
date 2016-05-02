#ifndef ERROR_H
#define ERROR_H

enum ServerErrors 
{
  //Generic Errors
  SUCCESS = 0,
  FAILURE,
  ALLOCATION_ERR,
  NULL_POINTER,
  THREAD_CREATION_ERR,

  // Socket Errors
  SOCKET_CREATION_ERR = 10,
  SOCKET_BIND_ERR,
  SOCKET_LISTEN_ERR,
  SOCKET_ACCEPT_ERR,
  
  // Client Messaging Errors
  MAX_CLIENTS_CONNECTED_ERR = 20,
};

#endif
