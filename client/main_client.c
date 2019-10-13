#include "client_commands.h"
#include "client_handlers.h"
#include "consts.h"
#include "helpers.h"

void parseArgs(int argc, char *argv[], char *port, char *server_IP);
int readCommand(char *buffer);
struct User *initUser();
int communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr);

int main(int argc, char *argv[]){

  // INITIALIZATION OF GLOBAL VARIABLES
  int udp_fd, n, size, status;

  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  struct User *user = initUser();
  char buffer[BUFFER_SIZE], commandArgs[COMMANDS][ARG_SIZE], *port, *server_IP, command[COMMAND_SIZE];

  port = (char *)malloc(16);
  if (port == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  server_IP = (char *)malloc(128);
  if (server_IP == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP socket
  hints.ai_flags = AI_NUMERICSERV;

  // Checks if any of the '-n' or '-p' flags was used
  parseArgs(argc, argv, port, server_IP);

  if (strlen(server_IP) == 0)
    server_IP = NULL;


  n = getaddrinfo(server_IP, port, &hints, &res);
  if (n != 0) {
    printf("Error connecting to server. Server: %s . Port: %s\n", server_IP, port);
    exit(1);
  }

  // Opens UDP socket
  udp_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (udp_fd == -1) {
    printf("1");
    exit(1);
  }

  scanf("%s", command);
  size = readCommand(buffer);

  // UDP
  while (strcmp(command, "exit") != 0) {

    if ((strcmp(command, "register") == 0) || (strcmp(command, "reg") == 0)) {
      status = registerUser(buffer, user);
      if (status == VALID) {
        communicateUDP(buffer, udp_fd, res, addr);
        handleRGR(buffer, user);
      } 
      else {
        memset(buffer, 0, BUFFER_SIZE);
      }
    }

    else if ((strcmp(command, "topic_list") == 0) ||
             (strcmp(command, "tl") == 0)) {
      status = topicList(buffer, user);
      if (status == VALID) {
        if(communicateUDP(buffer, udp_fd, res, addr)){
          parseCommand(buffer, commandArgs);
          handleLTR(commandArgs, user);
        }
      } 
      else {
        memset(buffer, 0, BUFFER_SIZE);
      }
    }

    else if (strcmp(command, "topic_select") == 0) {
      topicSelect(buffer, 0, user);
      memset(buffer, 0, BUFFER_SIZE);
    }

    else if (strcmp(command, "ts") == 0) {
      topicSelect(buffer, 1, user);
      memset(buffer, 0, BUFFER_SIZE);
    }

    else if ((strcmp(command, "topic_propose") == 0) ||
             (strcmp(command, "tp") == 0)) {
      status = topicPropose(buffer, user);
      if (status == VALID) {
        communicateUDP(buffer, udp_fd, res, addr);
        handlePTR(buffer, user);
      } else {
        memset(buffer, 0, BUFFER_SIZE);
      }
    }

    else if ((strcmp(command, "question_list") == 0) ||
             (strcmp(command, "ql") == 0)) {
      status = questionList(buffer, user);
      if (status == VALID) {
        // TODO: implement communication and handler
      } else {
        memset(buffer, 0, BUFFER_SIZE);
      }
    }

    else if (strcmp(command, "question_get") == 0) {
      questionGet(buffer, 0);
    }

    else if (strcmp(command, "qg") == 0) {
      questionGet(buffer, 1);
    }

    else if ((strcmp(command, "question_submit") == 0) ||
             (strcmp(command, "qs") == 0)) {
      questionSubmit(buffer);
    }

    else if ((strcmp(command, "answer_submit") == 0) ||
             (strcmp(command, "as") == 0)) {
      answerSubmit(buffer);
    }

    else {
      printf("Invalid command.\n");
    }

    memset(buffer, 0, BUFFER_SIZE);
    memset(command, 0, COMMAND_SIZE);
    scanf("%s", command);
    size = readCommand(buffer);
  }
  return 0;
}

void parseArgs(int argc, char *argv[], char *port, char *server_IP) {
  if (argc > 1) {
    int opt;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
      switch (opt) {
      case 'n':
        strcpy(server_IP, argv[2]);
        break;
      case 'p':
        strcpy(port, argv[4]);
        break;
      }
    }
  }
  if (strlen(port) == 0)
    strcpy(port, "58053");

  return;
}

int readCommand(char *buffer) {
  memset(buffer, 0, BUFFER_SIZE);
  char c = getchar();
  int i = 0;

  while (c != '\n') {
    buffer[i] = c;
    c = getchar();
    i++;
  }
  return i;
}

int communicateUDP(char *buffer, int fd, struct addrinfo *res,
                    struct sockaddr_in addr) {
  int nwrite, nread, size;
  socklen_t addrlen;

  size = strlen(buffer);

  nwrite = sendto(fd, buffer, size, 0, res->ai_addr, res->ai_addrlen);
  if (nwrite == -1) {
    exit(1);
  }

  memset(buffer, 0, BUFFER_SIZE);
  addrlen = sizeof(addr);

  nread = recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
  if (nread == -1) {
    exit(1);
  }

  size = strcspn(buffer, "\n");
  if( nread != (size + 1) ){
    printf("Error receiving message from server. New line character is mandatory.\n");
    return INVALID;
  }

  buffer[size] = '\0';
  return VALID;
}

struct User *initUser() {
  int i;

  struct User *user = (struct User *)malloc(sizeof(struct User));
  if (user == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->selected_question = (char *)malloc(10 * sizeof(char));
  if (user->selected_question == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->topics = (char **)malloc(99 * sizeof(char *));
  if (user->topics == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->userId = -1;
  
  user->selected_topic = (char*)malloc(10 * sizeof(char));
	if( user->selected_topic == NULL){
		printf("Error allocating memory.\n");
		exit(1);
	}

  for (i = 0; i < 99; i++) {
    user->topics[i] = (char *)malloc(10 * sizeof(char));
    if (user->topics[i] == NULL) {
      printf("Error allocating memory.\n");
      exit(1);
    }
  }

  return user;
}