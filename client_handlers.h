#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "consts.h"

void handleRGR(char *buffer, struct User *user);
void handleLTR(char *buffer, struct User *user);
void handlePTR(char *buffer, struct User *user);

#endif