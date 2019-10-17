#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include "../others/consts.h"
#include "../others/helpers.h"
#include "client_communication.h"

int registerUser(char *buffer, struct User *user, int numSpaces);
int topicList(char *buffer, struct User *user, int numSpaces);
void topicSelect(char *buffer, int flag, struct User *user, int numSpaces);
int topicPropose(char *buffer, struct User *user, char *topic, int numSpaces);
int questionList(char *buffer, struct User *user, int numSpaces);
int questionGet(char *buffer, int flag, struct User *user, char question[], int numSpaces);
int questionSubmit(struct User *user, char *commandArgs[], struct Submission* submission, int numSpaces);
int answerSubmit(struct User *user, char *commandArgs[], struct Submission* submission, int numSpaces);
int sendSubmission(struct User *user, struct Submission *submission, char *buffer, int tcp_fd, int type);

#endif