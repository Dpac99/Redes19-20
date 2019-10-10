#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include "consts.h"

int registerUser(char *buffer, struct User *user);
int topicList(char *buffer, struct User *user);
void topicSelect(char *buffer, int flag);
void topicPropose(char *buffer, struct User *user);
void questionList(char *buffer, struct User *user);
void questionGet(char *buffer, int flag);
void questionSubmit(char *buffer);
void answerSubmit(char *buffer);

#endif