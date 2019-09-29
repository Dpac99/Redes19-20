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
#define OK 						 "OK"
#define NOK 					 "NOK"
#define DUP 					 "DUP"
#define ERROR 					 "ERR"
#define END_OF_FILE 			 "EOF"


struct User{
	char userId[5];
	char selected_topic[10];		
	char selected_question[10];
};

void parseArgs(int argc, char *argv[], char *port, char *server_IP);
int readCommand(char *buffer);
void registerUser(char *buffer, int fd, struct addrinfo *res);
void topicList(char *buffer, int fd, struct addrinfo *res);
void topicSelect(char *buffer, int flag);
void topicPropose(char *buffer, int fd, struct addrinfo *res);
void questionList(char *buffer, int fd, struct addrinfo *res);
void questionGet(char *buffer, int flag);
void questionSubmit(char *buffer);
void answerSubmit(char *buffer);

int main(int argc, char *argv[]){

	int tcp_fd, udp_fd, addrlen;
	int nleft, nread, nwrite, n, errcode, size;

	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char buffer[256], *port, server_IP[128], command[64];

	port = (char*)malloc(16);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_DGRAM; // UDP socket
	hints.ai_flags=AI_NUMERICSERV;

	parseArgs(argc, argv, port, server_IP);

	n=getaddrinfo(server_IP, port, &hints, &res);
	if(n!=0){
		write(1, server_IP, strlen(server_IP));
		write(1, port, strlen(port));
        exit(1);
    }

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
			registerUser(buffer, udp_fd, res);
		}

		else if((strcmp(command, "topic_list") == 0)|| (strcmp(command, "tl") == 0)){
			topicList(buffer, udp_fd, res);
		}

		else if(strcmp(command, "topic_select") == 0){
			topicSelect(buffer, 0);
		}

		else if(strcmp(command, "ts") == 0){
			topicSelect(buffer, 1);
		}

		else if((strcmp(command, "topic_propose") == 0)|| (strcmp(command, "tp") == 0)){
			topicPropose(buffer, udp_fd, res);
		}

		else if((strcmp(command, "question_list") == 0)|| (strcmp(command, "ql") == 0)){
			questionList(buffer, udp_fd, res);
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

        nwrite=sendto(udp_fd, buffer, i, 0, res->ai_addr, res->ai_addrlen);
        if(nwrite==-1){
			printf("3");
            exit(1);
        }
		
		memset(buffer, 0, sizeof(char));
        addrlen=sizeof(addr);

        nread=recvfrom(udp_fd,buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
        if(nread==-1){
			printf("4");
            exit(1);
        }
	
        write(1, "received: ", 6);
		write(1, buffer, nread);
		printf("\n");

		memset(buffer, 0, sizeof(char));
		memset(command, 0, sizeof(char));
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
	if(strlen(server_IP) == 0) server_IP = NULL;
	return;
}

int readCommand(char *buffer){
	memset(buffer, 0, sizeof(char));
	char c = getchar();
	int i = 0;

	while(c != '\n'){
		buffer[i] = c;
		c = getchar();
		i++;
	}
	return i;
}

void registerUser(char *buffer, int fd, struct addrinfo *res){
}

void topicList(char *buffer, int fd, struct addrinfo *res){

}
void topicSelect(char *buffer, int flag){

}
void topicPropose(char *buffer, int fd, struct addrinfo *res){

}
void questionList(char *buffer, int fd, struct addrinfo *res){

}
void questionGet(char *buffer, int flag){

}
void questionSubmit(char *buffer){

}
void answerSubmit(char *buffer){

}