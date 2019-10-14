#include "client_handlers.h"
#include "helpers.h"

//PROTOCOL RESPONSE HANDLERS

void handleRGR(char *buffer, struct User *user){
	char *token;
	token = strtok(buffer, " ");

	if (strcmp(token, REGISTER_RESPONSE) == 0){
		token = strtok(NULL, " ");
		if(strcmp(token, OK) == 0){
			printf("User '%d' registered successfully.\n", user->userId);
		}
		else if(strcmp(token, NOK) == 0){
			printf("Failed to register user '%d'.\n", user->userId);
			user->userId = -1;
		}
	}
	else{
		printf("Error receiving answer from server.\n");
	}
	return;
}

int handleLTR(char *commandArgs[], struct User *user){
	char *token;
	char *UserIds[MAX_TOPICS][USER_ID_SIZE];
	int n_topics, i, err = 0;

	if((strcmp(commandArgs[0], TOPIC_LIST_RESPONSE) == 0) && (0 < isnumber(commandArgs[1]) <= MAX_TOPICS)){
		n_topics = atoi(commandArgs[1]);

		for(i = 0; i < n_topics; i++){
			token = strtok(commandArgs[i + 2], ":");
			if(isValidTopic(token)){
				token = strtok(NULL, ":");
				if(isnumber(token) && isValidId(token)){
					strcpy(user->topics[i], commandArgs[i+2]); 
					strcpy(UserIds[i], token);		
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
	else{
		err = 1;
	}

	if(err == 1){
		printf("Error receiving message from server.\n");
		for(i = 0; i < n_topics; i++){
			memset(user->topics[i], 0, TOPIC_SIZE);
		}
		return INVALID;
	}

	
	for(i = 0; i < n_topics; i++){
		printf("Topic %d: %s.	Proposed by user %s.\n", i+1, user->topics[i], UserIds[i]);
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

}