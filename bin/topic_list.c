void handleLTR(char *buffer, struct User *user){
	char *topic, *token;
	int n_topics, i, msg_size = strlen(buffer);
	int count = 0, err = 0;

	printf("%s", buffer);

	topic = strtok(buffer, " ");
	if(strcmp(topic, TOPIC_LIST_RESPONSE) != 0){					//Checks if the 1st word is 'LTR'
		printf("Error receiving answer from server1.\n");
		err = 1;
		return;
	}

	topic = strtok(NULL, " ");
	if(topic == NULL){												//Checks if the msg doesn't end after the 'LTR' protocol code
		printf("Error receiving answer from server2.\n");
		err = 1;
	}

	if(strcmp(topic, "0\n") == 0){									//Checks if there are available topics
		n_topics = atoi(topic);

		if(msg_size != 6){
			err = 1;
		}
		else if((topic = strtok(NULL, " ")) != NULL){				//If so, checks if the protocol msg ends correctly.
			err = 1;
		}
		else{
			printf("No available topics.\n");
			return;
		}
	}

	else if((n_topics = atoi(topic)) == 0){							//Checks if the 'N' parameter is a valid integer
		err = 1;
	}

	else{															//If so, adds the number of characters of n_topics.
		count += strlen(topic);
	}

	if((!err) && (n_topics > 0)){									// If there are topics to be shown:

		for(i = 0; i < n_topics; i++){
			printf("Aqui\n");

			topic = strtok(NULL, " ");
			printf("Topic: %s\n", topic);

			if(topic == NULL){											
				printf("Error receiving answer from server3.\n");
				err = 1;
			}

			else{
				
			}
		}
	}
	
	if((err == 1)) {							//If there's an error or the message hasn't ended after reading n_topics
		printf("Error receiving message from server4.\n");

		for(i = 0; i < n_topics; i++){
			memset(user->topics[i], 0, TOPIC_SIZE);
		}
	}

	else if( (count + n_topics + 4) != msg_size ){
		printf("Count: %d\n", count);
		printf("Error receiving message from server5.\n");			//If there's more ' ' than expected 
	}

	else{
		for(i = 0; i < n_topics; i++){
			printf("%d. %s - %s\n", i+1, topic, token);
		}
		user->num_topics = n_topics;
		
	}

	memset(buffer, 0, BUFFER_SIZE);
	return;
}