#ifndef __TCP_SERVER_HANDLERS_H__
#define __TCP_SERVER_HANDLERS_H__

int handleGetQuestion(int fd);
int handleSubmitQuestion(int fd);
int handleSubmitAnswer(int fd);

#endif