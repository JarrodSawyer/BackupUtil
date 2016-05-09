#ifndef __MESSAGES_H
#define __MESSAGES_H

typedef enum
{
  TEXT_MSG_ID = 0x0050 // Less than 100 == Debug message;
} MSG_ID;

typedef struct
{
  MSG_ID msgId;
  uint32_t bytesToFollow;
} Header;

#define TEXT_MAX_SIZE 256
typedef struct
{
  Header hdr;
  char text[TEXT_MAX_SIZE + 1]; // +1 to account for NULL terminator
} TextMessage;

typedef union
{
  Header hdr;
  TextMessage textMsg;
} Messages;

#endif
