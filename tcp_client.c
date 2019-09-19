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


	n=getaddrinfo("Mondego.tecnico.ulisboa.pt",PORT,&hints,&res);
	if(n!=0)/*error*/exit(1);

	fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(fd==-1)/*error*/exit(1);

	n=connect(fd,res->ai_addr,res->ai_addrlen);
	if(n==-1)/*error*/exit(1);

	while (1){

		scanf("%s", buffer);
		ptr=strcpy(buffer2, buffer);

		nleft = 15;
		while(nleft >0){
			nwrite=write(fd, buffer, nleft);
			if(nwrite==-1)/*error*/exit(1);
			else if (nwrite == 0) break;
			nleft-=nwrite;
			ptr+=nwrite;
		}

		memset(buffer, 0, sizeof(char));

		nleft=15; ptr = buffer;

		while(nleft>0){
			nread=read(fd,ptr,nleft);
			if(n==-1)/*error*/exit(1);
			else if (nread==0)break;
			nleft-=nread;
			ptr+=nread;
		}
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(fd);
}