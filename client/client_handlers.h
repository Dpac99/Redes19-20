#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "../others/consts.h"
#include "client_communication.h"
//UDP Handlers
int handleRGR(char *buffer, struct User *user);
int handleLTR(char *commandArgs[], struct User *user);
int handlePTR(char *buffer, struct User *user, char aux_topic[]);
int handleLQR(char *commandArgs[], struct User *user);

//TCP Handlers
int handleGQR(char *buffer, struct User *user, int tcp_fd);
int handleGQRAux(char *buffer, struct User* user, int tcp_fd, char *extra);
int handleQUR(char *buffer, struct User *user, int tcp_fd);
int handleANR(char *buffer, struct User *user, int tcp_fd);

#endif
