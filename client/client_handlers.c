#include "client_handlers.h"
#include "../others/helpers.h"

// PROTOCOL RESPONSE HANDLERS

int handleRGR(char *buffer, struct User *user) {
  char *token;
  token = strtok(buffer, " ");

  if (strcmp(token, REGISTER_RESPONSE) == 0) {
    token = strtok(NULL, " ");
    if (strcmp(token, OK) == 0) {
      printf("User '%d' registered successfully.\n", user->userId);
    } else if (strcmp(token, NOK) == 0) {
      printf("Failed to register user '%d'.\n", user->userId);
      user->userId = -1;
      return INVALID;
    }
  } else if (strcmp(token, ERROR) == 0) {
    printf("Server reported a fatal error.\n");
    return ERR;
  } else {
    printf("Error receiving answer from server.\n");
    return INVALID;
  }
  return VALID;
}

int handleLTR(char *commandArgs[], struct User *user) {
  char *token;
  char UserIds[MAX_TOPICS][USER_ID_SIZE + 1];
  int n_topics, i, err = 0;

  if ((strcmp(commandArgs[0], TOPIC_LIST_RESPONSE) == 0) &&
      isnumber(commandArgs[1])) {
    n_topics = atoi(commandArgs[1]);
    if ((0 < n_topics) && (n_topics <= MAX_TOPICS)) {
      for (i = 0; i < n_topics; i++) {
        token = strtok(commandArgs[i + 2], ":");
        if (isValidTopic(token)) {
          token = strtok(NULL, ":");
          if (isnumber(token) && isValidId(token)) {
            strcpy(user->topics[i], commandArgs[i + 2]);
            strcpy(UserIds[i], token);
          } else
            err = 1;
        } else
          err = 1;
        if (err == 1) {
          break;
        }
      }
    } else if (n_topics == 0) {
      if (strlen(commandArgs[2]) != 0) {
        err = 1;
      } else
        printf("No topics available yet.\n");
    } else
      err = 1;
  } else if (strcmp(commandArgs[0], ERROR) == 0) {
    err = -1;
  } else {
    err = 1;
  }

  if (err == 1) {
    printf("Error receiving message from server.\n");
    for (i = 0; i < n_topics + 2; i++) {
      memset(user->topics[i], 0, TOPIC_SIZE);
    }
    return INVALID;
  } else if (err == -1) {
    printf("Server reported a fatal error.\n");
    for (i = 0; i < n_topics + 2; i++) {
      memset(user->topics[i], 0, TOPIC_SIZE);
    }
    return ERR;
  }

  for (i = 0; i < n_topics; i++) {
    i < 9 ? printf("Topic 0%d: %10s\tProposed by user: %s.\n", i + 1,
                   user->topics[i], UserIds[i])
          : printf("Topic %d: %10s\tProposed by user: %s.\n", i + 1,
                   user->topics[i], UserIds[i]);
  }
  for (i = 0; i < COMMANDS; i++) {
    memset(commandArgs[i], 0, ARG_SIZE);
  }
  user->num_topics = n_topics;
  return VALID;
}

int handlePTR(char *buffer, struct User *user, char aux_topic[]) {
  char *token;
  int count = 0;
  int size = strlen(buffer);

  token = strtok(buffer, " ");
  count += strlen(token);

  if (strcmp(token, TOPIC_PROPOSE_RESPONSE) == 0) {
    token = strtok(NULL, " ");
    count += strlen(token);

    if (strcmp(token, OK) == 0) {
      strcpy(user->selected_topic, aux_topic);
      printf("Topic '%s' proposed successfully.\n", user->selected_topic);
    } else if (strcmp(token, NOK) == 0) {
      printf("Failed to propose topic '%s'.\n", aux_topic);
    } else if (strcmp(token, DUP) == 0) {
      printf("Topic '%s' already exists.\n", aux_topic);
    } else if (strcmp(token, FULL) == 0) {
      printf("Couldn't propose topic '%s'. There are already too many top\n",
             aux_topic);
    } else if (strcmp(token, ERROR) == 0) {
      printf("Server reported a fatal error.\n");
      return ERR;
    } else {
      printf("Error receiving answer from server.\n");
      return INVALID;
    }

    if ((size - count) > 1) {
      printf("Error receiving answer from server.\n");
      return INVALID;
    }
  } else {
    printf("Error receiving answer from server.\n");
    return INVALID;
  }
  return VALID;
}

int handleLQR(char *commandArgs[], struct User *user) {
  char *token;
  char UserIds[MAX_QUESTIONS][USER_ID_SIZE + 1];
  char NumAnswers[MAX_QUESTIONS][USER_ID_SIZE + 1];
  int n_questions, i, num, err = 0;

  if ((strcmp(commandArgs[0], QUESTION_LIST_RESPONSE) == 0) &&
      isnumber(commandArgs[1])) {
    n_questions = atoi(commandArgs[1]);

    if ((0 < n_questions) && (n_questions <= MAX_QUESTIONS)) {
      for (i = 0; i < n_questions; i++) {
        token = strtok(commandArgs[i + 2], ":");
        if (isValidTopic(token)) {
          token = strtok(NULL, ":");

          if (isnumber(token) && isValidId(token)) {
            strcpy(user->questions[i], commandArgs[i + 2]);
            strcpy(UserIds[i], token);
            token = strtok(NULL, ":");

            if (isnumber(token)) {
              num = atoi(token);
              if ((num >= 0) && (num <= MAX_ANSWERS))
                strcpy(NumAnswers[i], token);
              else
                err = 1;
            } else
              err = 1;
          } else {
            err = 1;
          }
        } else {
          err = 1;
        }

        if (err == 1) {
          break;
        }
      }
    } else if (n_questions == 0) {
      if (strlen(commandArgs[2]) == 0) {
        printf("No questions yet for topic '%s'.\n", user->selected_topic);
      } else {
        err = 1;
      }
    } else {
      err = 1;
    }
  } else if (strcmp(commandArgs[0], ERROR) == 0) {
    err = ERR;
  } else {
    err = 1;
  }

  if (err == 1) {
    printf("Error receiving message from server.\n");
    for (i = 0; i < n_questions + 2; i++) {
      memset(user->questions[i], 0, TOPIC_SIZE);
    }
    return INVALID;
  } else if (err == ERR) {
    printf("Server reported a fatal error.\n");
    for (i = 0; i < n_questions + 2; i++) {
      memset(user->questions[i], 0, TOPIC_SIZE);
    }
    return ERR;
  }

  for (i = 0; i < n_questions; i++) {
    printf("Question %d: %s.	Proposed by user %s with %s answers.\n", i + 1,
           user->questions[i], UserIds[i], NumAnswers[i]);
  }

  for (i = 0; i < COMMANDS; i++) {
    memset(commandArgs[i], 0, ARG_SIZE);
  }

  user->num_questions = n_questions;
  return VALID;
}

// QGR qUserID qsize qdata qIMG [qiext qisize qidata]
// N (AN aUserID asize adata aIMG [aiext aisize aidata])*
int handleGQR(char *buffer, struct User *user, int tcp_fd) {
	int i, status, numAnswers = 0;
	char command[4];
	char *an = (char*)malloc(4*sizeof(char));

	strcpy(an, "");

	// Read QGR, qUserID, qsize and qdata 
	memset(buffer, 0, BUFFER_SIZE);
	status = receiveTCP(buffer, 4, tcp_fd);
	if (status == ERR) {
		return ERR;
	} else if (status == 0) {
		printf("Couldn't receive message from server.\n");
		return INVALID;
	} else {
		for (i = 0; i < 3; i++)
			command[i] = buffer[i];
		command[i] = '\0';
		if (strcmp(command, GET_QUESTION_RESPONSE) != 0) {
			printf("Error receiving answer from server\n");
			return INVALID;
		}
	}
	status = handleGQRAux(buffer, user, tcp_fd, an);
	if (status != VALID)
		return status;

	// Ignore space
	memset(buffer, 0, BUFFER_SIZE);
	status = receiveTCP(buffer, 1, tcp_fd);
	if (status == ERR) {
		return ERR;
	} else if (status == 0) {
		printf("Couldn't receive message from server.\n");
		return INVALID;
	}

	// Read N
	memset(buffer, 0, BUFFER_SIZE);
	status = receiveTCP(buffer, 2, tcp_fd);
	if (buffer[1] == '\n') 
		buffer[1] = '\0';
	if (status == ERR) {
		return ERR;
	} else if (status == 0) {
		printf("Couldn't receive message from server.\n");
		return INVALID;
	} else {
		if (isnumber(buffer) == INVALID) {
			printf("Error receiving answer from server\n");
			return INVALID;
		}
		for (i = 0; i < strlen(buffer); i++) {
			numAnswers *= 10;
			numAnswers += buffer[i] - '0';
		}
	}
	if (numAnswers > 10 || numAnswers < 0) {
		printf("Error receiving answer from server\n");
		return INVALID;
	}

	if (numAnswers == 10) {
		// Ignore space
		memset(buffer, 0, BUFFER_SIZE);
		status = receiveTCP(buffer, 1, tcp_fd);
		if (status == ERR) {
			return ERR;
		} else if (status == 0) {
			printf("Couldn't receive message from server.\n");
			return INVALID;
		}
	}
	// Read files for each answer
	if (numAnswers != 0) {
		for (i = 0; i < numAnswers; i ++) {
			memset(buffer, 0, BUFFER_SIZE);
			status = receiveTCP(buffer, 3, tcp_fd);
			if (status == ERR) {
				return ERR;
			} else if (status == 0) {
				printf("Couldn't receive message from server.\n");
				return INVALID;
			} else {
				buffer[2] = '\0';	// Remove the space
				strcpy(an, "_");
				strcat(an, buffer);
				status = handleGQRAux(buffer, user, tcp_fd, an);
				if (status != VALID)
					return status;
			}
		}
	}

	return VALID;
}

int handleGQRAux(char *buffer, struct User* user, int tcp_fd, char *extra) {
	int qIMG, isize = 0, i, j, status, size = 0;
	char qUserID[6], sizeStr[10], ext[4];
	char dirname[128], filename[128];
	DIR *dir;
	FILE *fp;

	memset(buffer, 0, BUFFER_SIZE);
	status = receiveTCP(buffer, 17, tcp_fd);
	// Read QGR, qUserID, qsize and qdata 
	if (status == ERR) {
		return ERR;
	} else if (status == 0) {
		printf("Couldn't receive message from server.\n");
		return INVALID;
	} else {
		for (i = 0; i < 5; i++) {
			if (buffer[i] == ' ')
				break;
			qUserID[i] = buffer[i];
		}
		qUserID[i] = '\0';

		// Check if server sent errors
		if (strcmp(qUserID, END_OF_FILE) == 0) {
			printf("Question or topic does not exist\n");
			return INVALID;
		} else if (strcmp(qUserID, ERROR) == 0) {
			printf("Error receiving answer from server\n");
			return ERR;
		} else if (isnumber(qUserID) == INVALID || strlen(qUserID) != 5) {
			printf("Error receiving answer from server\n");
			return INVALID;
		}

		// Create topic directory
		strcpy(dirname, user->selected_topic);
		dir = opendir(dirname);
		if (dir == NULL) {
			status = mkdir(dirname, 0700);
			if (status < 0) {
				printf("Error creating directory %s.\n", dirname);
				return INVALID;
			}
		}

		i++;
		for (; i < 16; i++) {
			if (buffer[i] == ' ') {
				sizeStr[i-6] = '\0';	
				break;
			}
			sizeStr[i-6] = buffer[i];
		}
		i++;
		if (isnumber(sizeStr) == INVALID || strlen(sizeStr) > 10) {
			printf("Error receiving answer from server\n");
			return INVALID;
		}
		for (j = 0; j < strlen(sizeStr); j++) {
			size *= 10;
			size += sizeStr[j] - '0';
		}
		
		// Create file question.txt
		strcpy(filename, dirname);
		strcat(filename, "/");
		strcat(filename, user->aux_question);
		strcat(filename, extra);
		strcat(filename, ".txt");
		fp = fopen(filename, "w");
		if (fp == NULL) {
			deleteDir(dirname);
			printf("Error creating file %s.\n", filename);
			return INVALID;
		}

		// Start writing data in file
		for (; i < 17; i++) {
			if (size == 0) {
				buffer[i] = EOF;
				break;
			}
			fputc(buffer[i], fp);
			size--;
		}
	}

	// Read rest of qdata
	while (size >= BUFFER_SIZE) {
		memset(buffer, 0, BUFFER_SIZE);
		status = receiveTCP(buffer, BUFFER_SIZE, tcp_fd);
		if (status == ERR) {
			return ERR;
		} else if (status == 0) {
			printf("Couldn't receive message from server.\n");
			return INVALID;
		} else {
			for (i = 0; i < BUFFER_SIZE; i++) {
				fputc(buffer[i], fp);
			}
		}
		size -= BUFFER_SIZE;
	}
	if (size > 0) {
		memset(buffer, 0, BUFFER_SIZE);
		status = receiveTCP(buffer, size, tcp_fd);
		if (status == ERR) {
			return ERR;
		} else if (status == 0) {
			printf("Couldn't receive message from server.\n");
			return INVALID;
		} else if (size != 0){
			for (i = 0; i < size; i++) {
				fputc(buffer[i], fp);
			}
		}
	}
	fputc('\0', fp);
	fclose(fp);

	// Read image flag
	memset(buffer, 0, BUFFER_SIZE);
	status = receiveTCP(buffer, 3, tcp_fd);
	if (status == ERR) {
		return ERR;
	} else if (status == 0) {
		printf("Couldn't receive message from server.\n");
		return INVALID;
	} else if (buffer[0] != ' '  || (buffer[1] != '0' && buffer[1] != '1') || buffer[2] != ' '){
		printf("Couldn't receive message from server.\n");
		return INVALID;
	} else {
		qIMG = buffer[1];
	}

	// Read qiext, qisize and qimage
	if (qIMG == '1') {
		memset(buffer, 0, BUFFER_SIZE);
		status = receiveTCP(buffer, 15, tcp_fd);
		if (status == ERR) {
			return ERR;
		} else if (status == 0) {
			printf("Couldn't receive message from server.\n");
			return INVALID;
		} else {
			for (i = 0; i < 3; i++)
				ext[i] = buffer[i];
			ext[i] = '\0';

			i++;
			for (; i < 14; i++) {
				if (buffer[i] == ' ')
					break;
				sizeStr[i-4] = buffer[i];
			}
			sizeStr[i-4] = '\0';
			i++;
		}
		if (isnumber(sizeStr) == INVALID || strlen(sizeStr) > 10) {
			printf("Error receiving answer from server\n");
			return INVALID;
		}
		for (j = 0; j < strlen(sizeStr); j++) {
			isize *= 10;
			isize += sizeStr[j] - '0';
		}

		// Create file question.ext
		strcpy(filename, dirname);
		strcat(filename, "/");
		strcat(filename, user->aux_question);
		strcat(filename, extra);
		strcat(filename, ".");
		strcat(filename, ext);
		fp = fopen(filename, "w");
		if (fp == NULL) {
			deleteDir(dirname);
			printf("Error creating file %s.\n", filename);
			return INVALID;
		}

		// Start writing data in file
		for (; i < 15; i++) {
			if (isize == 0) {
				buffer[i] = EOF;
				break;
			}
			fputc(buffer[i], fp);
			isize--;
		}
		
		// Read rest of qidata
		while (isize >= BUFFER_SIZE) {
			memset(buffer, 0, BUFFER_SIZE);
			status = receiveTCP(buffer, BUFFER_SIZE, tcp_fd);
			if (status == ERR) {
				return ERR;
			} else if (status == 0) {
				printf("Couldn't receive message from server.\n");
				return INVALID;
			} else {
				for (i = 0; i < BUFFER_SIZE; i++) {
					fputc(buffer[i], fp);
				}
			}
			isize -= BUFFER_SIZE;
		}
		if (isize > 0) {
			memset(buffer, 0, BUFFER_SIZE);
			status = receiveTCP(buffer, isize, tcp_fd);
			if (status == ERR) {
				return ERR;
			} else if (status == 0) {
				printf("Couldn't receive message from server.\n");
				return INVALID;
			} else if (isize != 0){
				for (i = 0; i < isize; i++) {
					fputc(buffer[i], fp);
				}
			}
		}
		fputc(EOF, fp);
		fclose(fp);
	} 

	return VALID;
}

int handleQUR(char *buffer, struct User *user, int tcp_fd) {
  char *token;
  int count = 0, status, i;
  int size = strlen(buffer);

  status = receiveTCP(buffer, QUR_SIZE, tcp_fd);
  if (status == ERR) {
    return ERR;
    // printf("Received: '%s'\n", buffer);
  } else if (status != 0) {
    i = strcspn(buffer, "\n");
    if (status != (i + 1)) {
      printf("Error receiving message from server. New line character is "
             "mandatory.\n");
      return INVALID;
    }

    buffer[i] = '\0';
    token = strtok(buffer, " ");
    count += strlen(token);

    if (strcmp(token, SUBMIT_QUESTION_RESPONSE) == 0) {
      token = strtok(NULL, " ");
      count += strlen(token);

      if (strcmp(token, OK) == 0) {
        strcpy(user->selected_question, user->aux_question);
        printf("Question '%s' submited successfully.\n",
               user->selected_question);
      } else if (strcmp(token, NOK) == 0) {
        printf("Failed to submit question '%s'.\n", user->aux_question);
      } else if (strcmp(token, DUP) == 0) {
        printf("Question '%s' already exists in the topic '%s'.\n",
               user->aux_question, user->selected_topic);
      } else if (strcmp(token, FULL) == 0) {
        printf("Couldn't submit question '%s'. Topic '%s' can't accept more "
               "questions for now.\n",
               user->aux_question, user->selected_topic);
      } else if (strcmp(token, ERROR) == 0) {
        printf("Server reported a fatal error.\n");
        return ERR;
      } else {
        printf("Error receiving answer from server.\n");
        return INVALID;
      }

      if ((size - count) > 1) {
        printf("Error receiving answer from server.\n");
        return INVALID;
      }
    } else {
      printf("Error receiving answer from server.\n");
      return INVALID;
    }
  } else {
    printf("Couldn't receive message from server.\n");
    return INVALID;
  }

  return VALID;
}

int handleANR(char *buffer, struct User *user, int tcp_fd){
	char *token;
	int count = 0, status, i;
	int size = strlen(buffer);

	status = receiveTCP(buffer, QUR_SIZE, tcp_fd);
	if( status == ERR){
		return ERR;
        //printf("Received: '%s'\n", buffer);
    } else if( status != 0){
		i = strcspn(buffer, "\n");
		if (status != (i+ 1)) {
			printf("Error receiving message from server. New line character is "
				"mandatory.\n");
			return INVALID;
		}

 		buffer[i] = '\0';
		token = strtok(buffer, " ");
		count += strlen(token);

		if (strcmp(token, SUBMIT_ANSWER_RESPONSE) == 0){
			token = strtok(NULL, " ");
			count += strlen(token);

			if(strcmp(token, OK) == 0){
				printf("Answer to question '%s' submited successfully.\n", user->selected_question);
			}
			else if(strcmp(token, NOK) == 0){
				printf("Failed to submit answer to question '%s'.\n", user->selected_question);
			}
			else if(strcmp(token, FULL)){
				printf("Couldn't submit answer. Question '%s' can't accept more answers for now.\n",  user->selected_question);
			}
			else{
				printf("Error receiving answer from server.\n");
				return ERR;
			}
			
			if((size - count) > 1){
				printf("Error receiving answer from server.\n");
				return INVALID;
			}
		}
		else{
			printf("Error receiving answer from server.\n");
			return INVALID;
		}
	}
	return VALID;
}
