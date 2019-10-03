#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

//Commands
#define REGISTER 				 "REG"
#define REGISTER_RESPONSE 		 "RGR"
#define TOPIC_LIST 				 "LTP" 
#define TOPIC_LIST_RESPONSE 	 "LTR"
#define TOPIC_PROPOSE 			 "PTP"
#define TOPIC_PROPOSE_RESPONSE 	 "PTR"
#define QUESTION_LIST 			 "LQU"
#define QUESTION_LIST_RESPONSE 	 "LQR"
#define GET_QUESTION 			 "GQU"
#define GET_QUESTION_RESPONSE 	 "QGR"
#define SUBMIT_QUESTION 		 "QUS"
#define SUBMIT_QUESTION_RESPONSE "QUR"
#define SUBMIT_ANSWER 			 "ANS"
#define SUBMIT_ANSWER_RESPONSE 	 "ANR"
#define OK 						 "OK\n"
#define NOK 					 "NOK\n" 
#define DUP 					 "DUP"
#define ERROR 					 "ERR"
#define END_OF_FILE 			 "EOF"

#define BUFFER_SIZE				 256
#define COMMAND_SIZE			 64
#define VALID					 1
#define INVALID				 	 0

struct User{
	int userId;
	char *selected_topic;		
	char *selected_question;
};

void parseArgs(int argc, char *argv[], char *port, char *server_IP);
int readCommand(char *buffer);
struct User* initUser();
void communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr);


int registerUser(char *buffer, struct User *user);
void topicList(char *buffer, struct User *user);
void topicSelect(char *buffer, int flag);
void topicPropose(char *buffer, struct User *user);
void questionList(char *buffer, struct User *user);
void questionGet(char *buffer, int flag);
void questionSubmit(char *buffer);
void answerSubmit(char *buffer);

void handleRGR(char *buffer, struct User *user);
void handleLTR(char *buffer, struct User *user);
void handlePTR(char *buffer, struct User *user);

int main(int argc, char *argv[]){

	//INITIALIZATION OF GLOBAL VARIABLES
	int tcp_fd, udp_fd, n, size, status;

	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	struct User *user = initUser();
	char buffer[256], *port, *server_IP, command[COMMAND_SIZE];

	port = (char*)malloc(16);
	server_IP = (char*)malloc(128);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_DGRAM; // UDP socket
	hints.ai_flags=AI_NUMERICSERV;

	//Checks if any of the '-n' or '-p' flags was used
	parseArgs(argc, argv, port, server_IP);								
	if(strlen(server_IP)==0) server_IP = NULL;

	n=getaddrinfo(server_IP, port, &hints, &res);
	if(n!=0){
		printf("Server: %s . Port: %s\n", server_IP, port);
        exit(1);
    }

	//Opens UDP socket
	udp_fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);	
	if(udp_fd==-1){
		printf("1");
        exit(1);
    }
	
	scanf("%s", command);
	size = readCommand(buffer);

	// UDP
	while (strcmp(command, "exit") != 0){

		if((strcmp(command, "register") == 0)|| (strcmp(command, "reg") == 0)){
			status = registerUser(buffer, user);
			if (status == VALID){
				communicateUDP(buffer, udp_fd, res, addr);
				handleRGR(buffer, user);
			}
			else{
				memset(buffer, 0, BUFFER_SIZE);
			}
		}

		else if((strcmp(command, "topic_list") == 0)|| (strcmp(command, "tl") == 0)){
			topicList(buffer, user);
		}

		else if(strcmp(command, "topic_select") == 0){
			topicSelect(buffer, 0);
		}

		else if(strcmp(command, "ts") == 0){
			topicSelect(buffer, 1);
		}

		else if((strcmp(command, "topic_propose") == 0)|| (strcmp(command, "tp") == 0)){
			topicPropose(buffer, user);
		}

		else if((strcmp(command, "question_list") == 0)|| (strcmp(command, "ql") == 0)){
			questionList(buffer, user);
		}

		else if(strcmp(command, "question_get") == 0){
			questionGet(buffer, 0);
		}

		else if(strcmp(command, "qg") == 0){
			questionGet(buffer, 1);
		}

		else if((strcmp(command, "question_submit") == 0)|| (strcmp(command, "qs") == 0)){
			questionSubmit(buffer);
		}

		else if((strcmp(command, "answer_submit") == 0)|| (strcmp(command, "as") == 0)){
			answerSubmit(buffer);
		}
		
		else{
			printf("Invalid command.\n");
		}

		memset(buffer, 0, BUFFER_SIZE);
		memset(command, 0, COMMAND_SIZE);
		scanf("%s", command);
		size = readCommand(buffer);
	}
	return 0;
}

void parseArgs(int argc, char *argv[], char *port, char *server_IP){
	if (argc > 1){
		int opt;
		while((opt = getopt(argc, argv, "n:p:")) != -1) {  
        	switch(opt){  
            case 'n': 
				strcpy(server_IP, argv[2]);
				break;
            case 'p':  
				strcpy(port, argv[4]);
				break;
			} 
        }  
    }
	if(strlen(port)== 0) strcpy(port, "58053");
	
	return;
}

int readCommand(char *buffer){
	memset(buffer, 0, BUFFER_SIZE);
	char c = getchar();
	int i = 0;

	while(c != '\n'){
		buffer[i] = c;
		c = getchar();
		i++;
	}
	return i;
}

void communicateUDP(char *buffer, int fd, struct addrinfo *res, struct sockaddr_in addr){
	int nwrite, nread, size, addrlen;

	size = strlen(buffer);

	nwrite=sendto(fd, buffer, size, 0, res->ai_addr, res->ai_addrlen);
    if(nwrite==-1){
		exit(1);
	}
	memset(buffer, 0, BUFFER_SIZE);
	addrlen=sizeof(addr);

	nread=recvfrom(fd,buffer, BUFFER_SIZE, 0, (struct sockaddr*) &addr, &addrlen);
	if(nread==-1){
		printf("4");
		exit(1);
	}
	return;
}

struct User *initUser(){
	struct User *user = (struct User *)malloc(sizeof(struct User));
	user->userId = -1;
	user->selected_topic = (char*)malloc(10 * sizeof(char));
	user->selected_question = (char*)malloc(10 * sizeof(char));

	return user;
}

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
	sprintf(buffer, "%s %d", REGISTER, id);
	return VALID;
}

void topicList(char *buffer, struct User *user){

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

void  handleRGR(char *buffer, struct User *user){
	char *token;
	printf("Received: '%s' from server\n", buffer);
	token = strtok(buffer, " ");

	if (strcmp(token, REGISTER_RESPONSE) == 0){
		token = strtok(NULL, " ");
		if(strcmp(token, OK) == 0){
			printf("User '%d' registered.\n", user->userId);
		}
		else if(strcmp(token, NOK) == 0){
			user->userId = -1;
			printf("Failed to register user.\n");
		}
	}
	else{
		printf("Error receiving answer from server.\n");
	}
	return;
}