#ifndef __CONSTS_h__
#define __CONSTS_h__

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define PORT "58053"

// Commands


// UDP
#define REGISTER "REG"
#define REGISTER_RESPONSE "RGR"
#define TOPIC_LIST "LTP"
#define TOPIC_LIST_RESPONSE "LTR"
#define TOPIC_PROPOSE "PTP"
#define TOPIC_PROPOSE_RESPONSE "PTR"
#define QUESTION_LIST "LQU"
#define QUESTION_LIST_RESPONSE "LQR"

// TCP
#define GET_QUESTION "GQU"
#define GET_QUESTION_RESPONSE "QGR"
#define SUBMIT_QUESTION "QUS"
#define SUBMIT_QUESTION_RESPONSE "QUR"
#define SUBMIT_ANSWER "ANS"
#define SUBMIT_ANSWER_RESPONSE "ANR"

// Status codes
#define OK "OK"
#define NOK "NOK"
#define DUP "DUP"
#define ERROR "ERR"
#define END_OF_FILE "EOF"
#define FULL "FUL"

// Other info
#define TOPICS "TOPICS"
#define TRUE 1
#define FALSE 0
#define USER "USER.txt"
#define DATA "QUESTION_DATA"
#define Q_INFO "QUESTION_INFO.txt"
#define IMG "IMG"
#define IMG_DATA "IMG_DATA.txt"
#define ANS_DATA "ANS_DATA.txt"


#define BUFFER_SIZE				 	2048
#define COMMAND_SIZE			 	64
#define COMMANDS					128
#define ARG_SIZE					64
#define TOPIC_SIZE				 	10
#define MAX_TOPICS					99
#define MAX_QUESTIONS				99
#define USER_ID_SIZE				5
#define VALID					 	1
#define INVALID				 	 	0
#define REGISTER_REG_LENGTH		 	6	// after register/reg: " *****"
#define TOPIC_LIST_LENGTH		 	0 	// after topic_list/tl: ""
#define QUESTION_LIST_LENGTH		0 	// after question_list/ql: ""

struct User{
	int userId;
	char *selected_topic;		
	char *selected_question;
	char **topics;
	char **questions;
	int num_topics;
	int num_questions;
};

#endif