#include "client_commands.h"

/*************** COMMAND HANDLING**********************************/

////////////////// UDP COMMANDS ///////////////////////////////////

int registerUser(char *buffer, struct User *user, int numSpaces) {
  char *token;
  int id, n, count = 0;

  if (strlen(buffer) != REGISTER_REG_LENGTH || numSpaces != 1) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  token = strtok(buffer, " ");
  id = atoi(token);
  n = id;

  if (id == 0) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  while (n != 0) {
    n /= 10;
    ++count;
  }
  if (count != 5) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if ((token = strtok(NULL, " ")) != NULL) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  user->userId = id;
  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%s %d\n", REGISTER, id);
  return VALID;
}

int topicList(char *buffer, struct User *user, int numSpaces) {
  char *token;

  if (strlen(buffer) != TOPIC_LIST_LENGTH || numSpaces != 0) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if ((token = strtok(buffer, " ")) != NULL) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%s\n", TOPIC_LIST);
  return VALID;
}

void topicSelect(char *buffer, int flag, struct User *user, int numSpaces) {
  char *token, *topic;
  int num, buffer_size, i = 0, n_topics, status = INVALID;

  n_topics = user->num_topics;
  buffer_size = strlen(buffer);
  token = strtok(buffer, " ");

  if (token == NULL) {
    printf("Invalid command format.\n");
    return;
  }

  if ((buffer_size - strlen(token)) > 0 || numSpaces != 1) {
    printf("Invalid command format.\n");
    return;
  }

  if (n_topics > 0) {
    if (flag) { // input "ts num"
      if (isnumber(token)) {
        num = atoi(token);
        if (num <= 0 || num > MAX_TOPICS)
          printf("Invalid command format.\n");
        else {
          user->selected_topic = user->topics[num - 1];
          status = VALID;
        }
      }
    } else { // input "topic_select topic"
      topic = token;
      if (strlen(topic) > TOPIC_SIZE || (token = strtok(NULL, " ")) != NULL)
        printf("Invalid command format.\n");
      else {
        for (i = 0; i < n_topics; i++) {
          if (strcmp(user->topics[i], topic) == 0) {
            user->selected_topic = user->topics[i];
            status = VALID;
            break;
          }
        }
        if (status == INVALID)
          printf("The topic %s doesn't exist. Try again.\n", topic);
      }
    }
    if (status == VALID) {
      printf("Topic '%s' is now selected.\n", user->selected_topic);
    }
  }
  memset(buffer, 0, BUFFER_SIZE);
  return;
}

int topicPropose(char *buffer, struct User *user, char aux_topic[],
                 int numSpaces) {
  char *token, *topic;
  int buffer_size;

  if (user->userId == -1) {
    printf("Unregistered user.\n");
    return INVALID;
  }

  buffer_size = strlen(buffer);
  token = strtok(buffer, " ");

  if (token == NULL) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if ((buffer_size - strlen(token)) > 0 || numSpaces != 1) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  topic = token;
  if (strlen(topic) > TOPIC_SIZE || (token = strtok(NULL, " ")) != NULL)
    printf("Invalid command format.\n");

  strcpy(aux_topic, topic);
  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%s %d %s\n", TOPIC_PROPOSE, user->userId, aux_topic);

  return VALID;
}

int questionList(char *buffer, struct User *user, int numSpaces) {
  char *token;

  if (strlen(buffer) != QUESTION_LIST_LENGTH || numSpaces != 0) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if ((token = strtok(buffer, " ")) != NULL) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if (strlen(user->selected_topic) == 0) {
    printf("Please select a topic first.\n");
    return INVALID;
  }

  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%s %s\n", QUESTION_LIST, user->selected_topic);
  return VALID;
}

////////////////// TCP COMMANDS ///////////////////////////////////
int questionGet(char *buffer, int flag, struct User *user,
                int numSpaces) { // TODO: get question from the question list
  char *token;
  int i, status = INVALID, num, buffer_size, n_questions = user->num_questions;
  char *question;

  buffer_size = strlen(buffer);
  token = strtok(buffer, " ");

  if (token == NULL) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if ((buffer_size - strlen(token)) > 0 || numSpaces != 1) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if (flag) { // input "qg num"
    if (isnumber(token)) {
      num = atoi(token);
      if (num <= 0) {
        printf("Invalid command format.\n");
        return INVALID;
      }
      if (num > n_questions) {
        printf("Question number '%d' doesn't exist. Try again.\n", num);
      }
      question = user->questions[num - 1];
    } else {
      printf("Invalid command format.\n");
      return INVALID;
    }
  } else { // input "question_get topic"
    question = token;
    if (strlen(question) > QUESTION_SIZE ||
        (token = strtok(NULL, " ")) != NULL) {
      printf("Invalid command format.\n");
      return INVALID;
    }
    for (i = 0; i < n_questions; i++) {
      if (strcmp(user->questions[i], question) == 0) {
        question = user->questions[i];
        status = VALID;
        break;
      }
    }
    if (status == INVALID)
      printf("Question %s doesn't exist. Try again.\n", question);
  }
  strcpy(user->aux_question, question);
  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%s %s %s\n", GET_QUESTION, user->selected_topic,
          user->aux_question);
  // printf("buffer = %s", buffer);

  return VALID;
}

int questionSubmit(struct User *user, char *commandArgs[],
                   struct Submission *submission, int numSpaces) {
  int i;
  char *aux;
  char *question;
  char *text_file;
  char *image_file;
  char *ext;
  long file_size, image_size;

  // Allocate memory for filename and imagename
  char *filename = (char *)malloc(BUFFER_SIZE * sizeof(char));
  char *imagename = (char *)malloc(BUFFER_SIZE * sizeof(char));

  if (filename == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }
  if (imagename == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }

  // Check if the user is registered  and if there is a selected topic
  if (user->userId == -1) {
    printf("Unregistered user.\n");
    return INVALID;
  } else if (strcmp(user->selected_topic, "") == 0) {
    printf("No selected topic.\n");
    return INVALID;
  }

  // Check if there is a question in the input
  question = commandArgs[0];
  if (strlen(question) == 0) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  // Check if there is a text file in the input with name smaller than 10 chars
  text_file = commandArgs[1];
  if ((strlen(text_file) == 0) || strlen(question) > QUESTION_SIZE ||
      numSpaces != 1) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  // Check if file exists
  strcpy(filename, text_file);
  strcat(filename, ".txt");
  if (!fileExists(filename)) {
    printf("Text file or image file does not exist.\n");
    return INVALID;
  }

  // Check if there is an image in the input, if the format is image_file.ext
  // and if the file exists
  aux = commandArgs[2];
  if (strcmp(aux, "") != 0) {
    submission->imageExists = TRUE;
    strcpy(imagename, commandArgs[2]);
    image_file = strtok(aux, ".");
    ext = strtok(NULL, " ");
    if (image_file == NULL || ext == NULL) {
      printf("Invalid command format.\n");
      return INVALID;
    }
    if (!fileExists(imagename)) {
      printf("Text file or image file does not exist.\n");
      return INVALID;
    }
  } else {
    submission->imageExists = FALSE;
  }

  // Save selected question
  strcpy(user->aux_question, question);

  // Save text file name
  submission->text_name = (char *)malloc(strlen(filename) * sizeof(char));
  if (submission->text_name == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }
  file_size = fileSize(filename);
  if (file_size == -1) {
    return INVALID;
  }
  strcpy(submission->text_name, filename);
  submission->text_size = file_size;

  // Save image file name and extension
  if (submission->imageExists) {
    submission->image_name = (char *)malloc(strlen(imagename) * sizeof(char));
    if (submission->image_name == NULL) {
      printf("Error allocating memory.\n");
      return INVALID;
    }
    submission->image_ext = (char *)malloc(strlen(ext) * sizeof(char));
    if (submission->image_ext == NULL) {
      printf("Error allocating memory.\n");
      return INVALID;
    }
    image_size = fileSize(imagename);
    if (image_size == -1) {
      return INVALID;
    }
    strcpy(submission->image_name, imagename);
    strcpy(submission->image_ext, ext);
    submission->image_size = image_size;
  }

  // Clean commandArgs
  for (i = 0; i < COMMANDS; i++) {
    memset(commandArgs[i], 0, ARG_SIZE);
  }

  free(text_file);
  free(imagename);
  free(filename);
  free(aux);

  return VALID;
}

int answerSubmit(struct User *user, char *commandArgs[],
                 struct Submission *submission, int numSpaces) {
  int i;
  char *aux;
  char *text_file;
  char *image_file;
  char *ext;
  long file_size, image_size;

  // Allocate memory for filename and imagename
  char *filename = (char *)malloc(BUFFER_SIZE * sizeof(char));
  char *imagename = (char *)malloc(BUFFER_SIZE * sizeof(char));

  if (numSpaces != 1) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  if (filename == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }
  if (imagename == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }

  // Check if the user is registered  and if there is a selected topic and a
  // selected question
  if (user->userId == -1) {
    printf("Unregistered user.\n");
    return INVALID;
  } else if (strcmp(user->selected_topic, "") == 0) {
    printf("No selected topic.\n");
    return INVALID;
  } else if (strcmp(user->selected_question, "") == 0) {
    printf("No selected question.\n");
    return INVALID;
  }

  // Check if there is a text file in the input
  text_file = commandArgs[0];
  if (strlen(text_file) == 0) {
    printf("Invalid command format.\n");
    return INVALID;
  }

  // Check if the file exists
  strcpy(filename, text_file);
  strcat(filename, ".txt");
  if (!fileExists(filename)) {
    printf("Text file or image file does not exist.\n");
    return INVALID;
  }

  // Check if there is an image in the input, if the format is image_file.ext
  // and if the file exists
  aux = commandArgs[1];
  if (strcmp(aux, "") != 0) {
    submission->imageExists = TRUE;
    strcpy(imagename, commandArgs[1]);
    image_file = strtok(aux, ".");
    ext = strtok(NULL, " ");
    if ((image_file == NULL) || (ext == NULL)) {
      printf("Invalid command format.\n");
      return INVALID;
    }
    if (!fileExists(imagename)) {
      printf("Text file or image file does not exist.\n");
      return INVALID;
    }
  } else {
    submission->imageExists = FALSE;
  }

  // Save text file name
  submission->text_name = (char *)malloc(strlen(filename) * sizeof(char));
  if (submission->text_name == NULL) {
    printf("Error allocating memory.\n");
    return INVALID;
  }
  file_size = fileSize(filename);
  if (file_size == -1) {
    return INVALID;
  }
  strcpy(submission->text_name, filename);
  submission->text_size = file_size;

  if (submission->imageExists) {
    submission->image_name = (char *)malloc(strlen(imagename) * sizeof(char));
    if (submission->image_name == NULL) {
      printf("Error allocating memory.\n");
      return INVALID;
    }
    submission->image_ext = (char *)malloc(strlen(ext) * sizeof(char));
    if (submission->image_ext == NULL) {
      printf("Error allocating memory.\n");
      return INVALID;
    }
    image_size = fileSize(imagename);
    if (image_size == -1) {
      return INVALID;
    }
    strcpy(submission->image_name, imagename);
    strcpy(submission->image_ext, ext);
    submission->image_size = image_size;
  }

  // Clean commandArgs
  for (i = 0; i < COMMANDS; i++) {
    memset(commandArgs[i], 0, ARG_SIZE);
  }

  free(text_file);
  free(imagename);
  free(filename);
  free(aux);

  return VALID;
}

//QUS qUserID topic question qsize qdata qIMG [iext isize idata]
int sendSubmission(struct User *user, struct Submission *submission, char *buffer, int tcp_fd, int type){ //TODO: se der erro a meio, enviar ERR
  int i = 0, c, img_size = submission->image_size;
  FILE *fp;
  memset(buffer, 0, BUFFER_SIZE);

  if (type) {
    sprintf(buffer, "QUS %d %s %s %ld ", user->userId, user->selected_topic, user->aux_question,
            submission->text_size);
  } else {
    sprintf(buffer, "ANS %d %s %s %ld ", user->userId, user->selected_topic,
            user->selected_question, submission->text_size);
  }
  
  if(sendTCP(buffer, tcp_fd) == ERR) {
     printf("Error sending msg to server.\n");
     return ERR;
  }
  //printf("Sent: '%s'",buffer);
  
  fp = fopen(submission->text_name, "r");
  if (fp == NULL) {
    printf("Error opening file %s.\n", submission->text_name);
    return ERR;
  }

  memset(buffer, 0, BUFFER_SIZE);
  while (i < submission->text_size) {
    c = fgetc(fp);
    if (c == EOF)
      break;
    buffer[i] = c;
    if (i == (BUFFER_SIZE - 1)) {

      if (!sendTCP(buffer, tcp_fd)) {
        printf("Error sending msg to server.\n");
        return ERR;
      }
      //printf("'%s'",buffer);
      memset(buffer, 0, BUFFER_SIZE);
      i = -1;
    }
    i++;
  }

  if (!sendTCP(buffer, tcp_fd)) {
    printf("Error sending msg to server.\n");
    return ERR;
  }
  //printf("'%s'",buffer);
  memset(buffer, 0, BUFFER_SIZE);
  fclose(fp);

  if (!submission->imageExists) { // if there is no image
    sprintf(buffer, " 0\n");  
    if(sendTCP(buffer, tcp_fd) == ERR) {
      printf("Error sending msg to server.\n");
      return ERR;
    }
    //printf("'%s'",buffer);
  } else {
    fp = fopen(submission->image_name, "rb");
    if (fp == NULL) {
      printf("Error opening file %s.\n", submission->image_name);
      return ERR;
    }
    sprintf(buffer, " 1 %s %ld ", submission->image_ext, submission->image_size);
    if(sendTCP(buffer, tcp_fd) == ERR) {
      printf("Error sending msg to server.\n");
      return ERR;
    }
    //printf("'%s'",buffer);
    memset(buffer, 0, BUFFER_SIZE);
    i = 0;

    while (i < img_size) {
      c = fgetc(fp);
      buffer[i] = c;
      if (i == (BUFFER_SIZE-1)){
        
        if(sendTCP(buffer, tcp_fd) == ERR) {
          printf("Error sending msg to server.\n");
          return ERR;
        }
        //printf("'%s'",buffer);
        memset(buffer, 0, BUFFER_SIZE);
        img_size -= BUFFER_SIZE;
        i = -1;
      }
      i++;
    }
    if(sendTCP(buffer, tcp_fd) == ERR){
      printf("Error sending msg to server.\n");
      return ERR;
    }
    //printf("'%s'",buffer);

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "\n");
    if(sendTCP(buffer, tcp_fd) == ERR){
      printf("Error sending msg to server.\n");
      return ERR;
    }
    //printf("'%s'",buffer);
    fclose(fp);
  }

  memset(buffer, 0, BUFFER_SIZE);
  return VALID;
}