#include "../others/consts.h"
#include "../others/helpers.h"
#include "client_commands.h"
#include "client_handlers.h"
#include "client_communication.h"

void parseArgs(int argc, char *argv[], char *port, char *server_IP);
int readCommand(char *buffer, int *numSpaces);
void endClient(char **commandArgs, struct User *user, int udp_fd, char *buffer);
struct User *initUser();
struct Submission *initSubmission();



int main(int argc, char *argv[]) {

  // INITIALIZATION OF GLOBAL VARIABLES
  int udp_fd, n, status, i;
  int tcp_fd = 0;
  struct addrinfo hints, *res, *aux;
  struct sockaddr_in addr;
  struct User *user;
  struct Submission *submission;
  char *buffer, **commandArgs, *port, *server_IP, command[COMMAND_SIZE];
  int numSpaces = 0;
  char topic[TOPIC_SIZE];

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
  readCommand(buffer, &numSpaces);

  // UDP
  while (strcmp(command, "exit") != 0) {

    if ((strcmp(command, "register") == 0) || (strcmp(command, "reg") == 0)) {
      status = registerUser(buffer, user, numSpaces);
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
      status = topicList(buffer, user, numSpaces);
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
      topicSelect(buffer, 0, user, numSpaces);
      memset(buffer, 0, BUFFER_SIZE);
    }

    else if (strcmp(command, "ts") == 0) {
      topicSelect(buffer, 1, user, numSpaces);
      memset(buffer, 0, BUFFER_SIZE);
    }

    else if ((strcmp(command, "topic_propose") == 0) ||
             (strcmp(command, "tp") == 0)) {
      status = topicPropose(buffer, user, topic, numSpaces);
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

      status = questionList(buffer, user, numSpaces);
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
      status = questionGet(buffer, 0, user, numSpaces);
      if(status == VALID){
        printf("Question: '%s'.\n Sending: %s", user->aux_question, buffer);
      }
    
    }
    else if (strcmp(command, "qg") == 0) {
      status = questionGet(buffer, 1, user, numSpaces);
      if(status == VALID){
        if(connectTCP(res,aux, &tcp_fd)){
          if(sendTCP(buffer, tcp_fd)){
            
            memset(buffer, 0, BUFFER_SIZE);
  
            if(receiveTCP(buffer, BUFFER_SIZE, tcp_fd ) == VALID){
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
      // Clean commandArgs
      for (i = 0; i < COMMANDS; i++) {
        memset(commandArgs[i], 0, ARG_SIZE);
      }
      status = parseCommand(buffer, commandArgs);
      
      if (status == VALID)
        status = questionSubmit(user, commandArgs, submission, numSpaces);
      if(status == VALID){
        if(connectTCP(res,aux, &tcp_fd)){
          status = sendSubmission(user, submission, buffer, tcp_fd, 1);
          if(status == VALID){
            memset(buffer, 0, BUFFER_SIZE);
            handleQUR(buffer, user,  tcp_fd);  
          }
          else if(status == INVALID){
            printf("Error sending msg to server.\n");
          }
          else{
            endClient(commandArgs, user, udp_fd, buffer);
            exit(1);
          }
          if(tcp_fd > 0){
            close(tcp_fd);
          }
        }
      }
    }

    else if ((strcmp(command, "answer_submit") == 0) ||
             (strcmp(command, "as") == 0)) {
      // Clean commandArgs
      for (i = 0; i < COMMANDS; i++) {
        memset(commandArgs[i], 0, ARG_SIZE);
      }
      status = parseCommand(buffer, commandArgs);
      
      if (status == VALID) 
        status = answerSubmit(user, commandArgs, submission, numSpaces);
      if(status == VALID){
        if(connectTCP(res,aux, &tcp_fd)){
          if(sendSubmission(user, submission, buffer, tcp_fd, 0) == VALID){
            
            memset(buffer, 0, BUFFER_SIZE);
            if(receiveTCP(buffer, BUFFER_SIZE, tcp_fd) == VALID){
              printf("Received: '%s'\n", buffer);
            }

            else{
               printf("Receive_tcp failed.\n");
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

    else {
      printf("Invalid command.\n");
    }

    memset(buffer, 0, BUFFER_SIZE);
    memset(command, 0, COMMAND_SIZE);
    printf(">> ");
    scanf("%s", command);
    numSpaces = 0;
    readCommand(buffer, &numSpaces);
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

int readCommand(char *buffer, int *numSpaces) {
  memset(buffer, 0, BUFFER_SIZE);
  char c;
  int i = 0;

  while ((c = getchar()) != '\n') {
    if (!i && c == ' ') {
      *numSpaces += 1;
      continue; // Removes first space after command
    }
    buffer[i] = c;
    i++;
  }
  return i;
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

  user->aux_question = (char *)malloc(QUESTION_SIZE * sizeof(char));
  if (user->aux_question == NULL) {
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
  submission->imageExists = FALSE;
  submission->image_name = NULL;
  submission->image_content = NULL;

  return submission;
}

void endClient(char **commandArgs, struct User *user, int udp_fd, char *buffer){
  int i;
  free(user->selected_question);
  free(user->selected_topic);
  free(user->aux_question);
  
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


