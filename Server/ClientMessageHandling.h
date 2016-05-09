#ifndef CLIENT_MESSAGE_HANDLING_H
#define CLIENT_MESSAGE_HANDLING_H

void* initializeClientMessaging();
void* handleNewClientConnection(void *pContext, int clientFd);
int cleanupClientMessaging(void *pContext);
int waitForClientMessages(void *pContext, struct timeval *pTimeout);

#endif
