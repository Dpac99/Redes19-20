#include "../others/consts.h"
#include "../others/helpers.h"
#include "tcp_server_handlers.h"
#include "udp_server_handlers.h"

static volatile sig_atomic_t flag = 1;

void sigHandler(int _) {
  (void)_;
  flag = 0;
}

void parseArgs(int argc, char *argv[], char *port) {
  if (argc > 1) {
    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
      switch (opt) {
      case 'p':
        strcpy(port, argv[2]);
        break;
      }
    }
  }
  if (strlen(port) == 0)
    strcpy(port, PORT);

  return;
}

// Request Handlers
int udpHandler(char *buffer) {
  int size = strlen(buffer);
  char command[4], info[size - 3];
  int i;

  if (buffer[size - 1] != '\n') {
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "%s\n", ERROR);
    return 0;
  }

  for (i = 0; i < size; i++) {
    if (i < 3) {
      command[i] = buffer[i];
    } else if (i > 3) {
      info[i - 4] = buffer[i];
    } else if (i == 3) {
      command[i] = '\0';
    }
  }
  memset(buffer, 0, BUFFER_SIZE);

  if (strcmp(command, REGISTER) == 0) {
    return handleRegister(info, buffer);
  } else if (strcmp(command, TOPIC_LIST) == 0) {
    return handleTopicList(info, buffer);
  } else if (strcmp(command, TOPIC_PROPOSE) == 0) {
    return handleTopicPropose(info, buffer);
  } else if (strcmp(command, QUESTION_LIST) == 0) {
    return handleQuestionList(info, buffer);
  } else {
    sprintf(buffer, "%s\n", ERROR);
  }

  return 0;
}

int tcpHandler(char *command, int fd) {
  if (strcmp(command, GET_QUESTION) == 0) {
    return handleGetQuestion(fd);
  } else if (strcmp(command, SUBMIT_QUESTION) == 0) {
    return handleSubmitQuestion(fd);
  } else if (strcmp(command, SUBMIT_ANSWER) == 0) {
    return handleSubmitAnswer(fd);
  } else {
    printf("\tERR2\n");
    write(fd, "ERR\n", 4);
  }

  return 0;
}

int tcpCommunicate(int fd) {
  char command[8];

  int nread = 0;
  memset(command, 0, 8);

  printf("\tRECEIVED: ");
  while (nread < 4) {
    nread += read(fd, command + nread, 4 - nread);
  }

  printf("%s", command);

  if (command[3] != ' ') {
    printf("\tERR1\n");
    write(fd, "ERR\n", 4);
    return -1;
  }

  command[3] = 0;

  return tcpHandler(command, fd);
}

void initFS() {
  if (opendir(TOPICS) == NULL && errno == ENOENT) {
    mkdir(TOPICS, 0700);
  }
}

// Main
int main(int argc, char *argv[]) {
  struct sigaction act1;
  struct sigaction act2;
  struct sigaction act3;
  struct addrinfo hints, *res, *i;
  struct sockaddr_in addr;
  pid_t pid;
  socklen_t addrlen;
  int udp_fd = 0, tcp_fd = 0, errcode, maxfd, nready, resp_fd;
  ssize_t nread, nsent;
  char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE], host[BUFFER_SIZE],
      service[BUFFER_SIZE], port[16];
  fd_set rfds;

  parseArgs(argc, argv, port);

  act1.sa_handler = SIG_IGN;
  if (sigaction(SIGCHLD, &act1, NULL) == -1) {
    printf("Error with sigaction\n");
    exit(1);
  }

  act2.sa_handler = sigHandler;
  if (sigaction(SIGINT, &act2, NULL) == -1) {
    printf("Error with sigaction\n");
    exit(1);
  }

  act3.sa_handler = SIG_IGN;
  if (sigaction(SIGPIPE, &act3, NULL) == -1) {
    printf("Error with sigaction\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = 0; // Accepts TCP and UDP sockets
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  if ((errcode = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    printf("Error with getaddrinfo\n");
    exit(1);
  }

  printf("Server running on port %s\n\n", port);

  initFS();

  for (i = res; i != NULL; i = i->ai_next) {
    if (i->ai_socktype == SOCK_DGRAM) { // UDP Socket

      if ((udp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) ==
          -1) {
        if (tcp_fd > 0) {
          close(tcp_fd);
        }
        freeaddrinfo(i);
        if (flag)
          printf("Error with socket udp\n");
        exit(flag);
      }

      if (bind(udp_fd, i->ai_addr, i->ai_addrlen) == -1) {
        if (tcp_fd > 0) {
          close(tcp_fd);
        }
        freeaddrinfo(i);
        close(udp_fd);
        if (flag)
          printf("Error with bind udp\n");
        exit(flag);
      }

    } else if (i->ai_socktype == SOCK_STREAM) { // TCP Socket

      if ((tcp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) ==
          -1) {
        if (udp_fd > 0) {
          close(udp_fd);
        }
        freeaddrinfo(i);
        if (flag)
          printf("Error with socket tcp\n");
        exit(flag);
      }

      if (bind(tcp_fd, i->ai_addr, i->ai_addrlen) == -1) {
        freeaddrinfo(i);
        close(udp_fd);
        if (tcp_fd > 0) {
          close(tcp_fd);
        }
        if (flag)
          printf("Error with bind tcp: %d\n", errno);
        exit(flag);
      }

      if (listen(tcp_fd, SOMAXCONN) == -1) {
        if (udp_fd > 0)
          close(udp_fd);
        freeaddrinfo(i);
        close(tcp_fd);
        if (flag)
          printf("Error with listen tcp\n");
        exit(flag);
      }
    }
  }
  maxfd = max(tcp_fd, udp_fd) + 1;
  freeaddrinfo(res);

  while (flag) {
    FD_ZERO(&rfds);
    FD_SET(tcp_fd, &rfds);
    FD_SET(udp_fd, &rfds);

    nready = select(maxfd, &rfds, (fd_set *)NULL, (fd_set *)NULL,
                    (struct timeval *)NULL);
    if (nready < 0) {
      shutdown(resp_fd, SHUT_RDWR);
      close(tcp_fd);
      close(udp_fd);
      if (flag)
        printf("Error with select\n");
      exit(flag);
    }

    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    memset(buffer2, 0, sizeof(char) * BUFFER_SIZE);

    if (FD_ISSET(udp_fd, &rfds)) {
      nsent = 0;
      nread = 0;
      addrlen = sizeof(addr);
      nread = recvfrom(udp_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr,
                       &addrlen);

      if (nread == -1) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with recvfrom\n");
        exit(flag);
      }

      getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host),
                  service, sizeof service, 0);
      printf("[+]UDP Communication: Client[%s@%s]\n", host, service);
      printf("\tRECEIVED: %s", buffer);

      int err = udpHandler(buffer);

      int size = strlen(buffer);

      printf("\tSENT: %s", buffer);

      nsent =
          sendto(udp_fd, buffer, size, 0, (struct sockaddr *)&addr, addrlen);
      if (nsent == -1) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with sendto: %d\n", errno);
        exit(flag);
      }

      if (err != 0) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with handlers");
        exit(flag);
      }
      printf("[+]End of UDP with client[%s@%s]\n\n", host, service);
    }

    if (FD_ISSET(tcp_fd, &rfds)) {
      nsent = 0;
      nread = 0;
      addrlen = sizeof(addr);
      do
        resp_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addrlen);
      while (resp_fd == -1 && errno == EINTR);

      if ((pid = fork()) == -1) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with fork\n");
        exit(flag);
      }

      else if (pid == 0) { // Child process
        close(tcp_fd);
        if (resp_fd == -1) {
          if (flag)
            printf("Error with accept tcp\n");
          exit(flag);
        }
        getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host),
                    service, sizeof service, 0);
        printf("[+]TCP Communication: Client[%s@%s]\n", host, service);
        tcpCommunicate(resp_fd);
        shutdown(resp_fd, SHUT_RDWR);
        close(resp_fd);
        printf("[+]End of TCP with client[%s@%s]\n\n", host, service);
        exit(0);
      }

      else {
        int r;
        do
          r = close(resp_fd);
        while (r == -1 && errno == EINTR);
      }
    }
  }

  close(tcp_fd);
  close(udp_fd);
  return 0;
}
