#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "../others/consts.h"

//UDP Handlers
void handleRGR(char *buffer, struct User *user);
int handleLTR(char *commandArgs[], struct User *user);
int handlePTR(char *buffer, struct User *user, char aux_topic[]);
int handleLQR(char *commandArgs[], struct User *user);

//TCP Handlers
int handleQUR(char *buffer, struct User *user, char aux_question[]);
int handleANR(char *buffer, struct User *user);

#endif
