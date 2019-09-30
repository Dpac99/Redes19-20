#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "58053"
#define BUFFERSIZE 512

// Commands

// UDP
#define REGISTER "REG"
#define REGISTER_RESPONSE "RGR "
#define TOPIC_LIST "LTP"
#define TOPIC_LIST_RESPONSE "LTR "
#define TOPIC_PROPOSE "PTP"
#define TOPIC_PROPOSE_RESPONSE "PTR "
#define QUESTION_LIST "LQU"
#define QUESTION_LIST_RESPONSE "LQR "

// TCP
#define GET_QUESTION "GQU"
#define GET_QUESTION_RESPONSE "QGR "
#define SUBMIT_QUESTION "QUS"
#define SUBMIT_QUESTION_RESPONSE "QUR "
#define SUBMIT_ANSWER "ANS"
#define SUBMIT_ANSWER_RESPONSE "ANR "

// Status codes
#define OK "OK\n\0"
#define NOK "NOK\n\0"
#define DUP "DUP\n\0"
#define ERROR "ERR\n\0"
#define END_OF_FILE "EOF\n\0"
#define FULL "FUL\n\0"

// Other info
#define TOPICS "TOPICS"
#define TRUE 1
#define FALSE 0
#define USER "USER.txt"

static volatile sig_atomic_t flag = 1;

// Cache

struct cache {
  char **topics;
  int topic_n;
  int list_n;
};

struct cache *init() {
  struct cache *obj = (struct cache *)malloc(sizeof(struct cache));
  obj->topics = (char **)malloc(10 * sizeof(char *));
  if (obj->topics == NULL) {
    exit(1);
  }
  for (int i = 0; i < 10; i++) {
    obj->topics[i] = (char *)malloc(BUFFERSIZE * sizeof(char));
    if (obj->topics[i] == NULL) {
      exit(1);
    }
  }
  obj->topic_n = 0;
  obj->list_n = 0;
}

void freeCache(struct cache *obj) {
  for (int i = 0; i < 10; i++) {
    free(obj->topics[i]);
  }
  free(obj->topics);
  free(obj);
}

// Auxiliars

void sigHandler(int _) {
  (void)_;
  flag = 0;
}

int max(int x, int y) {
  if (x > y)
    return x;
  else
    return y;
}

void pushArr(char *s, char **arr, int size) {
  for (int i = size - 1; i > 0; i--) {
    strcpy(arr[i], arr[i - 1]);
  }
  strcpy(arr[0], s);
}

int comparator(const void *p, const void *q) {
  char p_c[32], q_c[32];
  struct stat p_s = {0}, q_s = {0};
  sprintf(p_c, "%s/%s", TOPICS, (char *)p);
  sprintf(q_c, "%s/%s", TOPICS, (char *)q);
  stat(p_c, &p_s);
  stat(q_c, &q_s);

  return (int)(p_s.st_mtim.tv_sec - q_s.st_mtim.tv_sec);
}

void loadFiles(struct cache *c) {
  DIR *dir;
  struct dirent *ent;
  char files[100][32];
  int i = 0;

  if ((dir = opendir(TOPICS)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        strcpy(files[i++], ent->d_name);
      }
    }
  }

  qsort((void *)files, i, 32 * sizeof(char), comparator);

  if (i > 9) {
    i = 9;
  }

  c->list_n = i;

  for (; i >= 0; i--) {
    strcpy(c->topics[9 - i], files[i]);
  }
}

// UDP Handlers
void handleRegister(char *info, char *dest) {
  int number;

  strcpy(dest, REGISTER_RESPONSE);

  number = atoi(info);

  if (number > 99999 || number < 10000) {
    strcat(dest, NOK);
    return;
  }

  strcat(dest, OK);
}

void handleTopicList(char *info, char *dest, struct cache *cache) {
  FILE *fd = NULL;
  char path[32];
  int id;
  sprintf(path, "%s/", TOPICS);

  strcpy(dest, TOPIC_LIST_RESPONSE);

  sprintf(dest + strlen(dest), "%d", cache->list_n);
  for (int i = 0; i < 10; i++) {
    if (strlen(cache->topics[i]) > 0) {
      sprintf(path + strlen(path), "%s/%s", cache->topics[i], USER);
      fd = fopen(path, "r");
      fscanf(fd, "%d", &id);
      sprintf(dest + strlen(dest), " %s:%d", cache->topics[i], id);
    }
  }
  strcat(dest, "\n");
  return;
}

void handleTopicPropose(char *info, char *dest, struct cache *cache) {
  char topic[16], dir[256], file[32];
  int uid = 0;
  struct stat st = {0};
  FILE *fd = NULL;

  sscanf(info, "%s %d", topic, &uid);
  strcpy(dest, TOPIC_PROPOSE_RESPONSE);
  if (strlen(topic) == 0 || uid == 0) {
    strcat(dest, NOK);
    return;
  }
  if (cache->topic_n == 99) {
    strcat(dest, FULL);
    return;
  }

  sprintf(dir, "%s/%s", TOPICS, topic);
  sprintf(file, "%s/%s", dir, USER);

  if (stat(dir, &st) == -1) {
    mkdir(dir, 0700);
    fd = fopen(file, "w");
    fprintf(fd, "%d", uid);
    fclose(fd);
    cache->topic_n++;
    if (cache->list_n < 10) {
      sprintf(cache->topics[9 - cache->list_n], "%s", topic);
      cache->list_n++;
    } else {
      pushArr(topic, cache->topics, 10);
    }
    strcat(dest, OK);
    return;
  } else {
    strcat(dest, DUP);
    return;
  }
}

void handleQuestionList(char *info, char *dest) {
  char path[32];
  char questions[100][32];
  DIR *dir;
  struct dirent *ent;
  int i = 0, id;
  FILE *fd = NULL;

  sprintf(path, "%s/%s", TOPICS, info);
  strcat(dest, QUESTION_LIST_RESPONSE);

  if ((dir = opendir(path)) != NULL) {
    while ((ent = readdir(dir)) != NULL && i < 100) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
          strcmp(ent->d_name, USER) != 0) {
        strcpy(questions[i++], ent->d_name);
      }
    }
    if (i > 99) {
      strcat(dest, FULL);
      return;
    }
  }

  sprintf(path + strlen(path), "/%s", USER);

  qsort((void *)questions, i, 32 * sizeof(char), comparator);

  i = i >= 10 ? 10 : i;

  sprintf(dest + strlen(dest), "%d", i);

  for (int s = 0; s < i; s++) {
    fd = fopen(path, "r");
    fscanf(fd, "%d", &id);
    fclose(fd);
    sprintf(dest + strlen(dest), " %s:%d", questions[s], id);
  }
  strcat(dest, "\n");
}

// Request Handlers
void handleUDP(char *buffer, struct cache *cache) {
  int size = strlen(buffer);
  char command[4], info[size - 3];
  int i, err;

  for (i = 0; i < size; i++) {
    if (i < 3) {
      command[i] = buffer[i];
    } else if (i > 3) {
      info[i - 4] = buffer[i];
    }else if (i == 3){
      command[i] = '\0';
    }
  }
  memset(buffer,0,  BUFFERSIZE);

  if (strcmp(command, REGISTER) == 0) {
    return handleRegister(info, buffer);
  } else if (strcmp(command, TOPIC_LIST) == 0) {
    return handleTopicList(info, buffer, cache);
  } else if (strcmp(command, TOPIC_PROPOSE) == 0) {
    return handleTopicPropose(info, buffer, cache);
  } else if (strcmp(command, QUESTION_LIST) == 0) {
    return handleQuestionList(info, buffer);
  } else {
    strcpy(buffer, ERROR);
  }
}

// Main
int main() {
  struct sigaction act1;
  struct sigaction act2;
  struct addrinfo hints, *res, *i;
  struct sockaddr_in addr;
  struct cache *cache;
  pid_t pid;
  socklen_t addrlen;
  int udp_fd = 0, tcp_fd = 0, errcode, maxfd, nready, resp_fd;
  ssize_t n, nread, nsent;
  char buffer[BUFFERSIZE], buffer2[BUFFERSIZE], *ptr;
  fd_set rfds;

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

  cache = init();
  loadFiles(cache);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = 0; // Accepts TCP and UDP sockets
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  if ((errcode = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
    printf("Error with getaddrinfo\n");
    exit(1);
  }

  for (i = res; i != NULL; i = i->ai_next) {
    if (i->ai_socktype == SOCK_DGRAM) { // UDP Socket

      if ((udp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) ==
          -1) {
        if (tcp_fd > 0) {
          close(tcp_fd);
        }
        freeaddrinfo(i);
        freeCache(cache);
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
        freeCache(cache);
        if (flag)
          printf("Error with bind udp\n");
        exit(flag);
      }

    } else if (i->ai_socktype == SOCK_STREAM) {

      if ((tcp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) ==
          -1) {
        if (udp_fd > 0) {
          close(udp_fd);
        }
        freeaddrinfo(i);
        freeCache(cache);
        if (flag)
          printf("Error with socket tcp\n");
        exit(flag);
      }

      if (bind(tcp_fd, i->ai_addr, i->ai_addrlen) == -1) {
        freeaddrinfo(i);
        close(udp_fd);
        if (tcp_fd > 0) {
          close(udp_fd);
        }
        freeCache(cache);
        if (flag)
          printf("Error with bind tcp\n");
        exit(flag);
      }

      if (listen(tcp_fd, SOMAXCONN) == -1) {
        if (udp_fd > 0)
          close(udp_fd);
        freeaddrinfo(i);
        close(tcp_fd);
        freeCache(cache);
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
      freeCache(cache);
      if (flag)
        printf("Error with select\n");
      exit(flag);
    }

    if (FD_ISSET(udp_fd, &rfds)) {
      memset(buffer, 0, sizeof(char) * BUFFERSIZE);
      memset(buffer2, 0, sizeof(char) * BUFFERSIZE);
      n, nsent, nread = 0;
      addrlen = sizeof(addr);
      nread = recvfrom(udp_fd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&addr,
                       &addrlen);

      if (nread == -1) {
        close(tcp_fd);
        close(udp_fd);
        freeCache(cache);
        if (flag)
          printf("Error with recvfrom\n");
        exit(flag);
      }

      printf("Received: %s\n", buffer);

      handleUDP(buffer, cache);

      int size = strlen(buffer);

      write(1,"SENT: ", 6);
      write(1 , buffer, size);

      nsent =
          sendto(udp_fd, buffer, size, 0, (struct sockaddr *)&addr, addrlen);
      if (nsent == -1) {
        close(tcp_fd);
        close(udp_fd);
        freeCache(cache);
        if (flag)
          printf("Error with sendto: %d\n", errno);
        exit(flag);
      }
    }

    if (FD_ISSET(tcp_fd, &rfds)) {
      memset(buffer, 0, sizeof(char) * BUFFERSIZE);
      memset(buffer2, 0, sizeof(char) * BUFFERSIZE);
      n, nsent, nread = 0;
      addrlen = sizeof(addr);
      resp_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addrlen);
      if (resp_fd == -1) {
        close(tcp_fd);
        close(udp_fd);
        freeCache(cache);
        if (flag)
          printf("Error with accept tcp\n");
        exit(flag);
      }

      if ((pid = fork()) == -1) {
        close(tcp_fd);
        close(udp_fd);
        freeCache(cache);
        if (flag)
          printf("Error with fork\n");
        exit(flag);
      } else if (pid == 0) { // Child process
        close(tcp_fd);
        while ((n = read(resp_fd, buffer2, 128)) != 0) {
          if (n == -1) {
            close(resp_fd);
            close(udp_fd);
            freeCache(cache);
            if (flag)
              printf("Error with read\n");
            exit(flag);
          }

          printf("Received: %s\n", buffer2);

          ptr = &buffer2[0];
          while (n > 0) {
            if ((nsent = write(resp_fd, ptr, n)) <= 0) {
              close(resp_fd);
              close(udp_fd);
              freeCache(cache);
              if (flag)
                printf("Error with write\n");
              exit(flag);
            }
            n -= nsent;
            ptr += nsent;
          }
        }
      }
    }
  }

  freeCache(cache);
  close(resp_fd);
  close(udp_fd);
  return 0;
}
