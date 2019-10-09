#ifndef __UDP_SERVER_HANDLERS_h__
#define __UDP_SERVER_HANDLERS_h__

void handleRegister(char *info, char *dest);
void handleTopicList(char *info, char *dest);
void handleTopicPropose(char *info, char *dest);
void handleQuestionList(char *info, char *dest);

#endif