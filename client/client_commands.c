#include "client_commands.h"

// COMMAND HANDLING

int registerUser(char *buffer, struct User *user){
	char *token;
	int id, n, count = 0;
	
	if (strlen(buffer) != REGISTER_REG_LENGTH) {
		printf("Invalid command format.\n");
		return INVALID;
	}
	token = strtok(buffer, " ");
	id = atoi(token);
	n = id;
	
	if(id == 0){
		printf("Invalid command format.\n");
		return INVALID;
	}
    while(n != 0){	
        n /= 10;
        ++count;
    }
	if(count != 5){
		printf("Invalid command format.\n");
		return INVALID;
	}
	if((token = strtok(NULL, " ")) != NULL){
		printf("Invalid command format.\n");
		return INVALID;
	}

	user->userId = id;
	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s %d\n", REGISTER, id);
	return VALID;
}

int topicList(char *buffer, struct User *user){
	char *token;

	if(strlen(buffer) != TOPIC_LIST_LENGTH) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	if((token = strtok(buffer, " ")) != NULL){
		printf("Invalid command format.\n");
		return INVALID;
	}

	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s\n", TOPIC_LIST);
	return VALID;
}

void topicSelect(char *buffer, int flag, struct User *user){		
	char *token, *topic;
	int num, buffer_size, i=0, n_topics, status = INVALID;

	n_topics = user->num_topics;
	buffer_size = strlen(buffer);
	token = strtok(buffer, " ");
	
	if(token == NULL) {
		printf("Invalid command format.\n");
		return;
	}

	if((buffer_size - strlen(token)) > 1) {
		printf("Invalid command format.\n");
		return;
	}

	if(n_topics > 0){
		if(flag) {	// input "ts num"
			if(isnumber(token)){
				num = atoi(token);
				if(num <= 0 || num > MAX_TOPICS)
					printf("Invalid command format.\n");
				else{
					user->selected_topic = user->topics[num - 1];
					status = VALID;
				}
			}
		}
		else {		// input "topic_select topic"
			topic = token;
			if(strlen(topic) > TOPIC_SIZE || (token = strtok(NULL, " ")) != NULL)
				printf("Invalid command format.\n");
			else{
				for(i = 0; i < n_topics; i++){
					if( strcmp(user->topics[i], topic) == 0){
						user->selected_topic = user->topics[i];
						status = VALID;
						break;
					}
				}
				if(status == INVALID)
					printf("The topic %s doesn't exist. Try again.\n", topic);
			}
		}
		if(status == VALID){
			printf("Topic '%s' is now selected.\n", user->selected_topic);
		}

	}
	else{
		printf("Please list the topics first.\n");
	}
	
	memset(buffer, 0, BUFFER_SIZE);
	return;
}

int topicPropose(char *buffer, struct User *user){					
	char *token, *topic;
	int buffer_size;

	buffer_size = strlen(buffer);
	token = strtok(buffer, " ");
	
	if(token == NULL) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	if((buffer_size - strlen(token)) > 1) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	topic = token;
	if(strlen(topic) > TOPIC_SIZE || (token = strtok(NULL, " ")) != NULL)
		printf("Invalid command format.\n");

	strcpy(user->selected_topic, topic);
	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s %s\n", TOPIC_PROPOSE, user->selected_topic);

	return VALID;
}

int questionList(char *buffer, struct User *user){
	char *token;

	if(strlen(buffer) != QUESTION_LIST_LENGTH) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	if((token = strtok(buffer, " ")) != NULL){
		printf("Invalid command format.\n");
		return INVALID;
	}

	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s\n", QUESTION_LIST);
	return VALID;
}

void questionGet(char *buffer, int flag){
}

void questionSubmit(char *buffer){
}

void answerSubmit(char *buffer){
}
