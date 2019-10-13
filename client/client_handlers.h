#ifndef __CLIENT_HANDLERS_H__
#define __CLIENT_HANDLERS_H__

#include "consts.h"

void handleRGR(char *buffer, struct User *user);
int handleLTR(char *commandArgs[], struct User *user);
void handlePTR(char *buffer, struct User *user);

#endif

/*
                token = strtok(topic, ":");								//Now further separates 'topic:userId' 
				if(topic == NULL){											
					printf("Error receiving answer from server.\n");
					err = 1;
				}
				printf("token: %s\n", token);

				if(strlen(token) > 11){									//Checks if topic length is greater than 10 caracthers
					err = 1;
					break;
				}
				strcpy(user->topics[i], token);
				count += strlen(topic);

				token = strtok(NULL, ":");
				printf("token: %s\n", token);
				if(topic == NULL){											
					printf("Error receiving answer from server.\n");
					err = 1;
				}

				if( (atoi(token) == 0) || (strlen(token) != 5) ){		//checks if userId is not a 5 character integer
					err = 1;
					break;
				}
			}

			if(err == 1){
				break;
			}
			
			/*topic = strtok(NULL, " ");
			if((topic == NULL) && (i != (n_topics - 1)) ){
				err = 1;
				break;
			}	*/