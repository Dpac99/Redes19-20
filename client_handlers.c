#include "client_handlers.h"

//PROTOCOL RESPONSE HANDLERS

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
	int n_topics, i, msg_size = strlen(buffer);
	int count = 0, err = 0;

	topic = strtok(buffer, " ");
	if( strcmp(topic, TOPIC_LIST_RESPONSE) != 0){					//Checks if the 1st word is 'LTR'
		printf("Error receiving answer from server.\n");
		err = 1;
		return;
	}

	topic = strtok(NULL, " ");
	if(topic == NULL){												//Checks if the msg doesn't end after the 'LTR' protocol code
		printf("Error receiving answer from server.\n");
		err = 1;
	}

	if(strcmp(topic, "0\n")){										//Checks if there are available topics
		n_topics = atoi(topic);

		if( msg_size != 5){
			printf("Error receiving answer from server.\n");
			err = 1;
		}
		else if((topic = strtok(NULL, " ")) != NULL){				//If so, checks if the protocol msg ends correctly.
			printf("Error receiving answer from server.\n");
			err = 1;
		}
		else{
			printf("No available topics.\n");
		}
	}

	else if((n_topics = atoi(topic) == 0)){							//Checks if the 'N' parameter is a valid integer
		printf("Error receiving answer from server.\n");
		err = 1;
	}

	else{															//If so, adds the number of characters of n_topics.
		count += strlen(topic);
	}

	if((!err) && (n_topics > 0)){									// If there are topics to be shown:

		topic = strtok(NULL, " ");
		if(topic == NULL){											
			printf("Error receiving answer from server.\n");
			err = 1;
		}

		for(i = 0; i < n_topics; i++){

			if((topic != NULL) && (!strcmp(topic, "\n"))){		
				token = strtok(topic, ":");								//Now further separates 'topic:userId' 
				if(strlen(token) > 10){									//Checks if topic length is greater than 10 caracthers
					err = 1;
					break;
				}

				token = strtok(NULL, ":");
				if( (atoi(token) == 0) || (strlen(token) != 5) ){		//checks if userId is not a 5 character integer
					err = 1;
					break;
				}
				if(!err){
					strcpy(user->topics[i], token);
					count += strlen(topic);
					topic = strtok(NULL, " ");
				}
			}

			if((topic == NULL) || (strcmp(topic, "\n")) ){				
				err = 1;
				break;
			}

			if(err){
				break;
			}
			
			topic = strtok(NULL, " ");
			if((topic == NULL) && (i != (n_topics - 1)) ){
				err = 1;
				break;
			}	
		}
	}
	
	if( err || (!(strcmp(topic, "\n"))) ) {							//If there's an error or the message hasn't ended after reading n_topics
		printf("Error receiving message from server.\n");

		for(i = 0; i < n_topics; i++){
			memset(user->topics[i], 0, TOPIC_SIZE);
		}
	}

	else if( (count + n_topics + 4) != msg_size ){
		printf("Error receiving message from server.\n");			//If there's more ' ' than expected 
	}

	else{
		for(i = 0; i < n_topics; i++){
			printf("%d. %s\n", i+1, topic, token);
		}
	}

	memset(buffer, 0, BUFFER_SIZE);
	return;
}