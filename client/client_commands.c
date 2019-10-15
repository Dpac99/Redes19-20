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

int topicPropose(char *buffer, struct User *user, char aux_topic[]){					
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

	strcpy(aux_topic, topic);
	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s %d %s\n", TOPIC_PROPOSE, user->userId, aux_topic);

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

	if(strlen(user->selected_topic) == 0){
		printf("Please select a topic first.\n");
		return INVALID;
	}

	memset(buffer, 0, BUFFER_SIZE);
	sprintf(buffer, "%s %s\n", QUESTION_LIST, user->selected_topic);
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

int questionSubmit(struct User *user, char *commandArgs[], struct Submission* submission){
	int i, imageExists = FALSE;
	char *aux;
	char *question;
	char *text_file;
	char *image_file;
	char *ext;
	char *filename = (char*)malloc(BUFFER_SIZE * sizeof(char));
	char *imagename = (char*)malloc(BUFFER_SIZE * sizeof(char));

	if (filename == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}
	if (imagename == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}

	if(strcmp(user->selected_topic, "") == 0) {
		printf("No selected topic.\n");
		return INVALID;
	}

	question = commandArgs[0];

	if(strlen(question) == 0) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	text_file = commandArgs[1];
	if((strlen(text_file) == 0) || strlen(question) > QUESTION_SIZE) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	strcpy(filename, text_file);
	strcat(filename, ".txt");
	if (!fileExists(filename)) {
		printf("Text file or image file does not exist.\n");
		return INVALID;
	}

	aux = commandArgs[2];
	if (strcmp(aux, "") != 0) {
		imageExists = TRUE;
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
	}
	strcpy(user->selected_question, question);

	for (i = 0; i < COMMANDS; i++) {
		memset(commandArgs[i], 0, ARG_SIZE);
	}

	// Save text file name and content
	aux = copyFile(filename);
	if (aux == NULL) {
		return INVALID;
	}
	submission->text_name = (char*)malloc(strlen(filename) * sizeof(char));
	if (submission->text_name == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}
	submission->text_content = (char*)malloc(strlen(aux) * sizeof(char));
	if (submission->text_content == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}
	strcpy(submission->text_name, filename);
	strcpy(submission->text_content, aux);

	// Save image file name and content
	if (imageExists)
		aux = copyFile(imagename);
	if (aux == NULL) {
		return INVALID;
	}
	submission->image_name = (char*)malloc(strlen(imagename) * sizeof(char));
	if (submission->image_name == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}
	submission->image_content = (char*)malloc(strlen(aux) * sizeof(char));
	if (submission->image_content == NULL) {
		printf("Error allocating memory.\n");
    	return INVALID;
	}
	strcpy(submission->image_name, imagename);
	strcpy(submission->image_content, aux);
	checkFileContent(submission);

	free(aux);
	free(filename);
	free(imagename);
	
	return VALID;
}

int answerSubmit(struct User *user, char *commandArgs[]){
	char *aux;
	char *text_file;
	char *image_file;
	char *ext;
	char *filename = (char*)malloc(BUFFER_SIZE * sizeof(char));
	char *imagename = (char*)malloc(BUFFER_SIZE * sizeof(char));

	if(strcmp(user->selected_topic, "") == 0) {
		printf("No selected topic.\n");
		return INVALID;
	} else if(strcmp(user->selected_question, "") == 0) {
		printf("No selected question.\n");
		return INVALID;
	}

	text_file = commandArgs[0];
	if(strlen(text_file) == 0) {
		printf("Invalid command format.\n");
		return INVALID;
	}

	strcpy(filename, text_file);
	strcat(filename, ".txt");
	if (!fileExists(filename)) {
		printf("Text file or image file does not exist.\n");
		return INVALID;
	}

	aux = commandArgs[1];
	if (strcmp(aux, "") != 0) {
		strcpy(imagename, commandArgs[1]);
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
	}

	return VALID;
}
