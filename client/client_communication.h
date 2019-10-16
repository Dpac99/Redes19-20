#ifndef __CLIENT_COMMUNICATIONS_H__
#define __CLIENT_COMMUNICATIONS_H__

#include "../others/consts.h"
#include "../others/helpers.h"


int communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr);
int connectTCP(struct addrinfo *res, struct addrinfo *aux, int *tcp_fd);
int sendTCP(char *buffer, int *tcp_fd);
int receiveTCP(char *buffer,  int msg_size, int *socket_fd);

#endif