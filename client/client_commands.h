#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include "../others/consts.h"

int registerUser(char *buffer, struct User *user);
int topicList(char *buffer, struct User *user);
void topicSelect(char *buffer, int flag, struct User *user);
int topicPropose(char *buffer, struct User *user, char *topic);
int questionList(char *buffer, struct User *user);
void questionGet(char *buffer, int flag, struct User *user);
int questionSubmit(char *buffer, struct User *user, char *commandArgs[]);
int answerSubmit(char *buffer);

#endif