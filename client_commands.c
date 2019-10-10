#include "client_commands.h"
#include "consts.h"

// COMMAND HANDLING

int registerUser(char *buffer, struct User *user){
	char *token;
	int id, count = 0, n;
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

	if((token = strtok(buffer, " ")) != NULL){
		printf("Invalid command format.\n");
		return INVALID;
	}

	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s\n", TOPIC_LIST);
	return VALID;
}

void topicSelect(char *buffer, int flag){
}

void topicPropose(char *buffer, struct User *user){
}

void questionList(char *buffer, struct User *user){
}

void questionGet(char *buffer, int flag){
}

void questionSubmit(char *buffer){
}

void answerSubmit(char *buffer){
}
