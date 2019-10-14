#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "../others/consts.h"

void handleRGR(char *buffer, struct User *user);
int handleLTR(char *commandArgs[], struct User *user);
int handlePTR(char *buffer, struct User *user, char aux_topic[]);
int handleLQR(char *commandArgs[], struct User *user);

#endif
