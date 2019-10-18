#include "client_communication.h"


int communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr){
  int nwrite, nread, size;
  socklen_t addrlen;

  size = strlen(buffer);

  nwrite = sendto(fd, buffer, size, 0, res->ai_addr, res->ai_addrlen);
  if (nwrite == -1) {
    return ERR;
  }

  memset(buffer, 0, BUFFER_SIZE);
  addrlen = sizeof(addr);

  nread =
      recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
  if (nread == -1) {
    return ERR;
  }

  size = strcspn(buffer, "\n");
  if (nread != (size + 1)) {
    printf("Error receiving message from server. New line character is "
           "mandatory.\n");
    return INVALID;
  }

  buffer[size] = '\0';

  return VALID;
}

int connectTCP(struct addrinfo *res, struct addrinfo *aux, int *tcp_fd){
  int fd = 0, n = 0;
  for (aux = res; aux != NULL; aux = aux->ai_next){
    if (aux->ai_socktype == SOCK_STREAM){                                     //TCP Socket
      fd = socket(aux->ai_family, aux->ai_socktype, aux->ai_protocol);
      if (fd == -1) {
        printf("Error establishing TCP socket connection\n");
        freeaddrinfo(res);
        freeaddrinfo(res->ai_next);
        return ERR;
      }
    }
  }
  
  n = connect(fd, res->ai_addr, res->ai_addrlen);
  if(n==-1){
    close(fd);
    return ERR;
  }
  
  *tcp_fd =fd;
  //printf("FILE DESCRIPTOR (connect): %d\n", *tcp_fd);
  return VALID;
}

int sendTCPText(char *buffer, int tcp_fd){
  int fd = tcp_fd;
  int nbytes, nleft, nwritten;
  char *ptr = buffer;
  
  nbytes = strlen(buffer);
  nleft = nbytes;
  //printf("FILE DESCRIPTOR (send): %d\n", tcp_fd);
  while(nleft > 0){
    nwritten=write(fd, ptr, nleft);
    if(nwritten <= 0){
      close(fd);
      return ERR;
    }
    nleft -= nwritten;
    ptr += nwritten;
  
  }
  return VALID;
}

int receiveTCP(char *buffer,  int msg_size, int tcp_fd){
  int nleft = msg_size;
  int nread = 1;
  int ntotal = 0;
  char *ptr = buffer;
  fd_set mask;
  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  FD_ZERO(&mask);
  FD_SET(tcp_fd, &mask);
  
  //printf("FILE DESCRIPTOR (receive): %d\n", tcp_fd);
  while((nleft>0) && select(tcp_fd + 1, &mask, NULL, NULL, &timeout)){ 
    nread = read(tcp_fd, ptr, nleft); 

    if(nread==-1){
      close(tcp_fd);
      return ERR;
    }
    else if(nread==0){
      break;//closed by peer
    } 

    nleft -= nread;
    ptr += nread;
    ntotal += nread;
    FD_ZERO(&mask);
    FD_SET(tcp_fd, &mask);
  }

  //printf("nread = %d\n", nread);
  return ntotal;
}