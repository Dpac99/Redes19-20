#include "client_handlers.h"
#include "consts.h"

void handleRGR(char *buffer, struct User *user){
	char *token;
	printf("Received: '%s' from server\n", buffer);
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

void handleLTR(char *buffer, struct User *user){
	char *topic;
	char *token;
	char *topic_aux;
	int err = 0;

	char *arr = (char*)malloc(strlen(buffer) * sizeof(char));
	if(arr == NULL){
		printf("Error allocating memory.\n");
		exit(1);
	}

	int count = 0, msg_size = strlen(buffer);

	topic = strtok(buffer, " ");
	if( strcmp(topic, TOPIC_LIST_RESPONSE) != 0){
		printf("Error receiving answer from server.\n");
		err = 1;
		return;
	}

	topic = strtok(NULL, " ");
	if(strcmp(topic, "0\n")){
		printf("No available topics.\n");
		if((topic = strtok(NULL, " ")) != NULL){			//checks if the 1st word is "LTR"
			printf("Error receiving answer from server.\n");
			err = 1;
		}
			printf("Error receiving answer from server.\n");
			err = 1;
		}
		return;
	}

	while((topic != NULL) && (!strcmp(topic, "\n"))){
		token = strtok(topic, ":");
		if(strlen(token) > 10){								//checks if topic length is greater than 10 caracthers
			printf("Error receiving answer from server.\n");
			err = 1;
		}

		token = strtok(NULL, ":");

		if( (atoi(token) == 0) || (strlen(token) != 5) ){	//checks if userId is not a 5 character integer
			printf("Error receiving answer from server.\n");
			err = 1;
		}
		topic = strtok(NULL, " ");
	}

	return;
}