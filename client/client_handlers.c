#include "client_handlers.h"
#include "../others/helpers.h"

// PROTOCOL RESPONSE HANDLERS

void handleRGR(char *buffer, struct User *user) {
  char *token;
  token = strtok(buffer, " ");

  if (strcmp(token, REGISTER_RESPONSE) == 0) {
    token = strtok(NULL, " ");
    if (strcmp(token, OK) == 0) {
      printf("User '%d' registered successfully.\n", user->userId);
    } else if (strcmp(token, NOK) == 0) {
      printf("Failed to register user '%d'.\n", user->userId);
      user->userId = -1;
    }
  } else {
    printf("Error receiving answer from server.\n");
  }
  return;
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
  } else {
    err = 1;
  }

  if (err == 1) {
    printf("Error receiving message from server.\n");
    for (i = 0; i < n_topics + 2; i++) {
      memset(user->topics[i], 0, TOPIC_SIZE);
    }
    return INVALID;
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

int handlePTR(char *buffer, struct User *user, char aux_topic[]){
	char *token;
	int count = 0;
	int size = strlen(buffer);

	token = strtok(buffer, " ");
	count += strlen(token);

	if (strcmp(token, TOPIC_PROPOSE_RESPONSE) == 0){
		token = strtok(NULL, " ");
		count += strlen(token);

		if(strcmp(token, OK) == 0){
			strcpy(user->selected_topic, aux_topic);
			printf("Topic '%s' proposed successfully.\n", user->selected_topic);
		}
		else if(strcmp(token, NOK) == 0){
			printf("Failed to propose topic '%s'.\n", aux_topic);
		}
		else if(strcmp(token, DUP) == 0){
			printf("Topic '%s' already exists.\n", aux_topic);
		}
		else if(strcmp(token, FULL)){
			printf("Couldn't propose topic '%s'. There are already too many top\n", aux_topic);
		}
		else{
			printf("Error receiving answer from server.\n");
			return INVALID;
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
	return VALID;
}

int handleLQR(char *commandArgs[], struct User *user){
	char *token;
	char UserIds[MAX_QUESTIONS][USER_ID_SIZE +1];
	char NumAnswers[MAX_QUESTIONS][USER_ID_SIZE + 1];
	int n_questions, i, num, err = 0;

	if((strcmp(commandArgs[0], QUESTION_LIST_RESPONSE) == 0) && isnumber(commandArgs[1])){
		n_questions = atoi(commandArgs[1]);

		if( (0 < n_questions ) && (n_questions <= MAX_QUESTIONS)){
			for(i = 0; i < n_questions; i++){
				token = strtok(commandArgs[i + 2], ":");
				if(isValidTopic(token)){
					token = strtok(NULL, ":");

					if(isnumber(token) && isValidId(token)){
						strcpy(user->questions[i], commandArgs[i+2]); 
						strcpy(UserIds[i], token);	
						token = strtok(NULL, ":");

						if(isnumber(token)){
							num = atoi(token);
							if((num >= 0) && (num <= MAX_ANSWERS))
								strcpy(NumAnswers[i], token);
							else
								err = 1;
						}
						else
							err = 1;
					}
					else{
						err = 1;
					}
				}
				else{
					err = 1;
				}

				if( err == 1){
					break;
				}	
			}
		}
		else if( n_questions == 0){
			if( strlen(commandArgs[2]) == 0){
				printf("No questions yet for topic '%s'.\n", user->selected_topic);
			}
			else{
				err = 1;
			}
		}
		else{
			err = 1;
		}
	}
	else{
		err = 1;
	}

	if(err == 1){
		printf("Error receiving message from server.\n");
		for(i = 0; i < n_questions + 2; i++){
			memset(user->questions[i], 0, TOPIC_SIZE);
		}
		return INVALID;
	}

	
	for(i = 0; i < n_questions; i++){
		printf("Question %d: %s.	Proposed by user %s with %s answers.\n", i+1, user->questions[i], UserIds[i], NumAnswers[i]);
	}

	for(i = 0; i < COMMANDS; i++){
		memset(commandArgs[i], 0, ARG_SIZE);
	}

	user->num_questions = n_questions;
	return VALID;
}

int handleQUR(char *buffer, struct User *user, char aux_question[]){
	char *token;
	int count = 0;
	int size = strlen(buffer);

	token = strtok(buffer, " ");
	count += strlen(token);

	if (strcmp(token, SUBMIT_QUESTION_RESPONSE) == 0){
		token = strtok(NULL, " ");
		count += strlen(token);

		if(strcmp(token, OK) == 0){
			strcpy(user->selected_question, aux_question);
			printf("Question '%s' submited successfully.\n", user->selected_question);
		}
		else if(strcmp(token, NOK) == 0){
			printf("Failed to submit quesiton '%s'.\n", aux_question);
		}
		else if(strcmp(token, DUP) == 0){
			printf("Question '%s' already exists in the topic '%s'.\n", aux_question, user->selected_topic);
		}
		else if(strcmp(token, FULL)){
			printf("Couldn't submit question '%s'. Topic '%s' can't accept more questions for now.\n", aux_question, user->selected_topic);
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
	return VALID;
}

int handleANR(char *buffer, struct User *user){
	char *token;
	int count = 0;
	int size = strlen(buffer);

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
	return VALID;
}