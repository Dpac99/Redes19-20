#ifndef __TCP_SERVER_HANDLERS_H__
#define __TCP_SERVER_HANDLERS_H__

int parseGetQuestion(char *info, char *topic, char *question);
void handleGetQuestion(char *info, char *dest);

#endif