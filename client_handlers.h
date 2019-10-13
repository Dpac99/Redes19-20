#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "consts.h"

void handleRGR(char *buffer, struct User *user);
int handleLTR(char *commandArgs[], struct User *user);
int handlePTR(char *buffer, struct User *user);
int handleLQR(char *commandArgs[], struct User *user);

#endif
