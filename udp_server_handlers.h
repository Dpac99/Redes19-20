#ifndef __UDP_SERVER_HANDLERS_h__
#define __UDP_SERVER_HANDLERS_h__

int handleRegister(char *info, char *dest);
int handleTopicList(char *info, char *dest);
int handleTopicPropose(char *info, char *dest);
int handleQuestionList(char *info, char *dest);

#endif