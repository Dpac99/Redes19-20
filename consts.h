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

#define PORT "58053"
#define BUFFERSIZE 2048

// Commands


// UDP
#define REGISTER "REG"
#define REGISTER_RESPONSE "RGR "
#define TOPIC_LIST "LTP"
#define TOPIC_LIST_RESPONSE "LTR "
#define TOPIC_PROPOSE "PTP"
#define TOPIC_PROPOSE_RESPONSE "PTR "
#define QUESTION_LIST "LQU"
#define QUESTION_LIST_RESPONSE "LQR "

// TCP
#define GET_QUESTION "GQU"
#define GET_QUESTION_RESPONSE "QGR "
#define SUBMIT_QUESTION "QUS"
#define SUBMIT_QUESTION_RESPONSE "QUR "
#define SUBMIT_ANSWER "ANS"
#define SUBMIT_ANSWER_RESPONSE "ANR "

// Status codes
#define OK "OK\n\0"
#define NOK "NOK\n\0"
#define DUP "DUP\n\0"
#define ERROR "ERR\n\0"
#define END_OF_FILE "EOF\n\0"
#define FULL "FUL\n\0"

// Other info
#define TOPICS "TOPICS"
#define TRUE 1
#define FALSE 0
#define USER "USER.txt"
#define DATA "QUESTION_DATA"
#define INFO "QUESTION_INFO.txt"
#define IMG "IMG"
#define IMG_DATA "IMG_DATA.txt"
#define ANS_INFO "ANS_DATA.txt"

#endif