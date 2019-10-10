#ifndef __CONSTS_H__
#define __CONSTS_H__

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

#define BUFFER_SIZE				 2048
#define COMMAND_SIZE			 64
#define VALID					 1
#define INVALID				 	 0

struct User{
	int userId;
	int selected_topic;		
	char *selected_question;
	char **topics;
};

#endif