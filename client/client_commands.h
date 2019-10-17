#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include "../others/consts.h"
#include "../others/helpers.h"
#include "client_communication.h"

int registerUser(char *buffer, struct User *user);
int topicList(char *buffer, struct User *user);
void topicSelect(char *buffer, int flag, struct User *user);
int topicPropose(char *buffer, struct User *user, char *topic);
int questionList(char *buffer, struct User *user);
int questionGet(char *buffer, int flag, struct User *user, char question[]);
int questionSubmit(struct User *user, char *commandArgs[], struct Submission* submission);
int answerSubmit(struct User *user, char *commandArgs[], struct Submission* submission);
int sendSubmission(struct User *user, struct Submission *submission, char *buffer, int tcp_fd, int type);

#endif