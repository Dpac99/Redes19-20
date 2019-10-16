#include "../others/consts.h"
#include "../others/helpers.h"
#include "client_commands.h"
#include "client_handlers.h"

void parseArgs(int argc, char *argv[], char *port, char *server_IP);
int readCommand(char *buffer);
void endClient(char **commandArgs, struct User *user, int udp_fd, char *buffer);
struct User *initUser();
struct Submission *initSubmission();
int communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr);
int connectTCP(struct addrinfo *res, struct addrinfo *aux, int *tcp_fd);
int sendTCP(char *buffer, int *tcp_fd);
int receiveTCP(char *buffer,  int msg_size, int *socket_fd);


int main(int argc, char *argv[]) {

  // INITIALIZATION OF GLOBAL VARIABLES
  int udp_fd, n, status, i;
  int tcp_fd = 0;
  struct addrinfo hints, *res, *aux;
  struct sockaddr_in addr;
  struct User *user;
  struct Submission *submission;
  char *buffer, **commandArgs, *port, *server_IP, command[COMMAND_SIZE];
  char topic[TOPIC_SIZE], question[QUESTION_SIZE];

  user = initUser();
  submission = initSubmission();
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
  hints.ai_socktype = 0; // Both UDP and TCP socket
  hints.ai_flags = AI_NUMERICSERV;

  // Checks if any of the '-n' or '-p' flags was used
  parseArgs(argc, argv, port, server_IP);

  if (strlen(server_IP) == 0)
    server_IP = NULL;

  commandArgs = (char **)malloc(COMMANDS * sizeof(char *));
  if (commandArgs == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  for (i = 0; i < COMMANDS; i++) {
    commandArgs[i] = (char *)malloc(ARG_SIZE * sizeof(char));
    if (commandArgs[i] == NULL) {
      printf("Error allocating memory.\n");
      exit(1);
    }
  }

  buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
  if (buffer == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  n = getaddrinfo(server_IP, port, &hints, &res);
  if (n != 0) {
    printf("Error connecting to server. Server: %s . Port: %s\n", server_IP,
           port);
    exit(1);
  }

  // Opens Sockets
  for (aux = res; aux != NULL; aux = aux->ai_next){

    if (aux->ai_socktype == SOCK_DGRAM){                                     //UDP Socket
      udp_fd = socket(aux->ai_family, aux->ai_socktype, aux->ai_protocol);

      if (udp_fd == -1) {
        printf("Error with UDP socket connection\n");
        freeaddrinfo(res);
        freeaddrinfo(res->ai_next);
        endClient(commandArgs, user, udp_fd, buffer);
        exit(1);
      }
    }
  }

  printf("Welcome!\n>> ");
  scanf("%s", command);
  readCommand(buffer);

  // UDP
  while (strcmp(command, "exit") != 0) {

    if ((strcmp(command, "register") == 0) || (strcmp(command, "reg") == 0)) {
      status = registerUser(buffer, user);
      if (status == VALID) {
        status = communicateUDP(buffer, udp_fd, res, addr);
        if( status == VALID){
          handleRGR(buffer, user);
        }
        else if(status == ERR){
          endClient(commandArgs, user, udp_fd, buffer);
          exit(1);
        }
      }
    }

    else if ((strcmp(command, "topic_list") == 0) ||
             (strcmp(command, "tl") == 0)) {
      status = topicList(buffer, user);
      if (status == VALID) {

        status = communicateUDP(buffer, udp_fd, res, addr);
        if (status == VALID) {
          parseCommand(buffer, commandArgs);
          handleLTR(commandArgs, user);
        }
        else if(status == ERR){
          endClient(commandArgs, user, udp_fd, buffer);
          exit(1);
        }
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
      status = topicPropose(buffer, user, topic);
      if (status == VALID) {
        status = communicateUDP(buffer, udp_fd, res, addr);
        if(status == VALID){
          handlePTR(buffer, user, topic);
        }
        else if(status == ERR){
          endClient(commandArgs, user, udp_fd, buffer);
          exit(1);
        }
      }
    }

    else if ((strcmp(command, "question_list") == 0) ||
             (strcmp(command, "ql") == 0)) {

      status = questionList(buffer, user);
      if (status == VALID) {
        status = communicateUDP(buffer, udp_fd, res, addr);
        if(status == VALID){
          parseCommand(buffer, commandArgs);
           handleLQR(commandArgs, user);
        }
        else if(status == ERR){
          endClient(commandArgs, user, udp_fd, buffer);
          exit(1);
        }
      }
    }

    else if (strcmp(command, "question_get") == 0) {
      status = questionGet(buffer, 0, user, question);
      if(status == VALID){
        printf("Question: '%s'.\n Sending: %s", question, buffer);
      }
    
    }

    else if (strcmp(command, "qg") == 0) {
      status = questionGet(buffer, 1, user, question);
      if(status == VALID){
        if(connectTCP(res,aux, &tcp_fd)){
          if(sendTCP(buffer, &tcp_fd)){
            
            memset(buffer, 0, BUFFER_SIZE);
  
            if(receiveTCP(buffer, 10, &tcp_fd )){
              printf("Received: '%s'\n", buffer);
              receiveTCP(buffer, 10, &tcp_fd);
              printf("Received: '%s'\n", buffer);
            }
            else{
              printf("Error receiving msg from server.\n");
            }
          }
          else{
            printf("Error sending msg to server.\n");
          }
          if(tcp_fd > 0){
            close(tcp_fd);
          }
        }
      }
    }

    else if ((strcmp(command, "question_submit") == 0) ||
             (strcmp(command, "qs") == 0)) {
      parseCommand(buffer, commandArgs);
      status = questionSubmit(user, commandArgs, submission);
    }

    else if ((strcmp(command, "answer_submit") == 0) ||
             (strcmp(command, "as") == 0)) {
      parseCommand(buffer, commandArgs);
      status = answerSubmit(user, commandArgs, submission);
    }

    else {
      printf("Invalid command.\n");
    }

    memset(buffer, 0, BUFFER_SIZE);
    memset(command, 0, COMMAND_SIZE);
    printf(">> ");
    scanf("%s", command);
    readCommand(buffer);
  }
  endClient(commandArgs, user, udp_fd, buffer);
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
  char c;
  int i = 0;

  while ((c = getchar()) != '\n') {
    if (!i && c == ' ') {
      continue; // Removes first space after command
    }
    buffer[i] = c;
    i++;
  }
  return i;
}

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

struct User *initUser() {
  int i;

  struct User *user = (struct User *)malloc(sizeof(struct User));
  if (user == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->selected_question = (char *)malloc(QUESTION_SIZE * sizeof(char));
  if (user->selected_question == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->topics = (char **)malloc(MAX_TOPICS * sizeof(char *));
  if (user->topics == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->questions = (char **)malloc(MAX_QUESTIONS * sizeof(char *));
  if (user->topics == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  user->userId = -1;
  user->num_topics = 0;
  user->num_questions = 0;

  user->selected_topic = (char *)malloc(10 * sizeof(char));
  if (user->selected_topic == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  for (i = 0; i < MAX_TOPICS; i++) {
    user->topics[i] = (char *)malloc(TOPIC_SIZE * sizeof(char));
    if (user->topics[i] == NULL) {
      printf("Error allocating memory.\n");
      exit(1);
    }

    user->questions[i] = (char *)malloc(TOPIC_SIZE * sizeof(char));
    if (user->questions[i] == NULL) {
      printf("Error allocating memory.\n");
      exit(1);
    }
  }
  return user;
}

struct Submission *initSubmission() {
  struct Submission *submission = (struct Submission *)malloc(sizeof(struct Submission));
  if (submission == NULL) {
    printf("Error allocating memory.\n");
    exit(1);
  }

  submission->text_name = NULL;
  submission->text_content = NULL;
  submission->image_name = NULL;
  submission->image_content = NULL;

  return submission;
}

void endClient(char **commandArgs, struct User *user, int udp_fd, char *buffer){
  int i;
  free(user->selected_question);
  free(user->selected_topic);
  
  if(udp_fd > 0){
    close(udp_fd);
  }

  for(i = 0; i < MAX_TOPICS; i++){
    free(user->topics[i]);
    free(user->questions[i]);
  }

  free(user->topics);
  free(user->questions);

  for(i = 0; i < MAX_TOPICS; i++){
    free(commandArgs[i]);
  }

  free(commandArgs);
  free(buffer);
  return;
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
  return VALID;
}

int sendTCP(char *buffer, int *tcp_fd){
  int fd = *tcp_fd;
  int nbytes, nleft, nwritten;
  char *ptr = buffer;
  
  nbytes = strlen(buffer);
  nleft = nbytes;
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

int receiveTCP(char *buffer,  int msg_size, int *tcp_fd){
  int nleft = msg_size;
  int nread = 1;
  char *ptr = buffer;
  

  while(nleft>0){ 
    nread=read(*tcp_fd, ptr, nleft); 
    if(nread==-1){
      close(*tcp_fd);
      return ERR;
    }
    else if(nread==0) break;//closed by peer
    nleft-=nread;
    ptr+=nread;
  }
  return VALID;
}