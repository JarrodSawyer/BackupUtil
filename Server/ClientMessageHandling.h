#ifndef CLIENT_MESSAGE_HANDLING_H
#define CLIENT_MESSAGE_HANDLING_H

void* initializeClientMessaging();
int handleNewClientConnection(void *pContext, int clientFd);
int cleanupClientMessaging(void *pContext);

#endif
