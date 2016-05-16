#ifndef CLIENT_MESSAGE_HANDLING_H
#define CLIENT_MESSAGE_HANDLING_H

void* handleNewClientConnection(int *pClientFd);
int cleanupClientMessaging(void *pContext);
int waitForClientMessages(void *pContext, struct timeval *pTimeout);

#endif
