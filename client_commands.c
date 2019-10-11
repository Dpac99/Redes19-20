#include "client_commands.h"
#include "consts.h"

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

void topicSelect(char *buffer, int flag, struct User *user){		//TODO: confirmar se e preciso fazer free do topic
	char *token;
	int num, buffer_size;
	char* topic;

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

	if(flag) {	// input "ts num"
		num = atoi(token);
		if(num <= 0 || num > 99)
			printf("Invalid command format.\n");
	}
	else {		// input "topic_select topic"
		topic = token;
		if(strlen(topic) > 10 || (token = strtok(NULL, " ")) != NULL)
			printf("Invalid command format.\n");
		user->selected_topic = topic;
	}

	memset(buffer, 0, BUFFER_SIZE);
}

int topicPropose(char *buffer, struct User *user){					//TODO: confirmar se e preciso fazer free do topic
	char *token;
	int num, buffer_size;
	char* topic;

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
	if(strlen(topic) > 10 || (token = strtok(NULL, " ")) != NULL)
		printf("Invalid command format.\n");
	user->selected_topic = topic;

	memset(buffer, 0, BUFFER_SIZE);
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
	sprintf(buffer, "%s\n", TOPIC_LIST);
	return VALID;
}

void questionGet(char *buffer, int flag){
}

void questionSubmit(char *buffer){
}

void answerSubmit(char *buffer){
}
