#include "client_commands.h"
#include "helpers.h"

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
	int buffer_size;
	char *topic;

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
	if(strlen(topic) > TOPIC_SIZE || (token = strtok(NULL, " ")) != NULL) {
		printf("Invalid command format.\n");
		return INVALID;
	}
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

void questionGet(char *buffer, int flag, struct User *user){	//TODO: get question from the question list
	char *token;
	int num, buffer_size;
	char* question;

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

	if(flag) {	// input "qg num"
		num = isnumber(token);
		if(num <= 0 || num > MAX_QUESTIONS) {
			printf("Invalid command format.\n");
			return;
		}
	}
	else {		// input "question_get topic"
		question = token;
		if(strlen(question) > QUESTION_SIZE || (token = strtok(NULL, " ")) != NULL) {
			printf("Invalid command format.\n");
			return;
		}
		user->selected_question = question;
	}

	memset(buffer, 0, BUFFER_SIZE);
}

int questionSubmit(char *buffer, struct User *user, char *commandArgs[]){	//TODO: clean code and check number of spaces
	char *aux;
	int buffer_size;
	char *question;
	char *text_file;
	char *image_file;
	char *ext;
	char *filename = (char*)malloc(BUFFER_SIZE * sizeof(char));
	char *imagename = (char*)malloc(BUFFER_SIZE * sizeof(char));
	int image_ext_size = 0;

	buffer_size = strlen(buffer);
	question = commandArgs[0];

	if(question == NULL) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	text_file = commandArgs[1];
	if(text_file == NULL || strlen(question) > QUESTION_SIZE) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	strcpy(filename, text_file);
	strcat(filename, ".txt");
	if (!fileExists(filename)) {
		printf("Text file does not exist.\n");
		return INVALID;
	}

	aux = commandArgs[2];
	if (aux != NULL) {
		strcpy(imagename, commandArgs[2]);
		image_ext_size = strlen(aux);
		image_file = strtok(aux, ".");
		ext = strtok(NULL, " ");
		if (image_file == NULL || ext == NULL) {
			printf("Invalid command format.\n");
			return INVALID;
		}
		if (!fileExists(imagename)) {
		printf("Image file does not exist.\n");
		return INVALID;
	}
	}

	// if((buffer_size - strlen(question) - strlen(text_file) - image_ext_size) > 2 && image_ext_size == 0) {
	// 	printf("Invalid command format.\n");
	// 	return INVALID;
	// } else if((buffer_size - strlen(question) - strlen(text_file) - image_ext_size) > 3) {
	// 	printf("Invalid command format.\n");
	// 	return INVALID;
	// }



	strcpy(user->selected_question, question);

	return VALID;
}

int answerSubmit(char *buffer){
}
