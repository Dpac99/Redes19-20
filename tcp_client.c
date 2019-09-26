#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#define PORT "58000"

int main(){
	int fd, addrlen, nleft, nread, nwrite,n, errcode; 
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char *ptr,buffer[128], buffer2[128];


	memset(&hints,0,sizeof hints);
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_STREAM; // UDP socket
	hints.ai_flags=AI_NUMERICSERV;


	n=getaddrinfo(NULL,PORT,&hints,&res);
	if(n!=0)/*error*/exit(1);

	fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(fd==-1)/*error*/exit(1);

	n=connect(fd,res->ai_addr,res->ai_addrlen);
	if(n==-1)/*error*/exit(1);

	while (1){

		scanf("%s", buffer);
		
		nwrite=write(fd, buffer, strlen(buffer));
		if(nwrite==-1)/*error*/exit(1);
	

		memset(buffer, 0, sizeof(char));

		nread=read(fd,buffer2,15);
		if(n==-1)/*error*/exit(1);
		write(1, "Received: ",10);
		write(1, buffer2, strlen(buffer2));
		write(1, "\n", 1);
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(fd);
}