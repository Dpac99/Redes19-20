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

int tcpHandler(char *buffer) {
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
  info[i - 4] = '\0';
  memset(buffer, 0, BUFFER_SIZE);

  if (strcmp(command, GET_QUESTION) == 0) {
    printf("Submitting\n");
    return handleGetQuestion(info, buffer);
  } else if (strcmp(command, SUBMIT_QUESTION) == 0) {
    return handleSubmitQuestion(info, buffer);
  } else if (strcmp(command, SUBMIT_ANSWER) == 0) {
    return handleSubmitAnswer(info, buffer);
  } else {
    sprintf(buffer, "%s\n", ERROR);
  }

  return 0;
}

void tcpCommunicate(int sockfd) {
  int n, nsent;
  char buffer[BUFFER_SIZE], *ptr;
  memset(buffer, 0, BUFFER_SIZE);
  n = recv(sockfd, buffer, BUFFER_SIZE, 0);
  if (n == -1) {
    close(sockfd);
    if (flag)
      printf("Error with read: %d\n", errno);
    exit(flag);
  }

  printf("Received: %s", buffer);

  tcpHandler(buffer);
  n = strlen(buffer);

  ptr = &buffer[0];
  printf("SENT: %s", buffer);

  while (n > 0) {
    if ((nsent = write(sockfd, ptr, n)) <= 0) {
      close(sockfd);
      if (flag)
        printf("Error with write: %d\n", errno);
      exit(flag);
    }
    n -= nsent;
    ptr += nsent;
  }
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
  struct addrinfo hints, *res, *i;
  struct sockaddr_in addr;
  pid_t pid;
  socklen_t addrlen;
  int udp_fd = 0, tcp_fd = 0, errcode, maxfd, nready, resp_fd;
  ssize_t nread, nsent;
  char buffer[BUFFER_SIZE], buffer2[BUFFER_SIZE], host[BUFFER_SIZE],
      service[BUFFER_SIZE], *port;
  fd_set rfds;

  port = (char *)malloc(16);
  if (port == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

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

  act2.sa_handler = sigHandler;
  if (sigaction(SIGINT, &act2, NULL) == -1) {
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
      printf("Received: %sfrom[%s:%s]\n\n", buffer, host, service);

      int err = udpHandler(buffer);

      int size = strlen(buffer);

      write(1, "SENT: ", 6);
      write(1, buffer, size);

      nsent =
          sendto(udp_fd, buffer, size, 0, (struct sockaddr *)&addr, addrlen);
      if (nsent == -1) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with sendto: %d\n", errno);
        exit(flag);
      }
      write(1, "\n", 1);

      if (err != 0) {
        close(tcp_fd);
        close(udp_fd);
        if (flag)
          printf("Error with handlers");
        exit(flag);
      }
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
        tcpCommunicate(resp_fd);
        close(resp_fd);
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
