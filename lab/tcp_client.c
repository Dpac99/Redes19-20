#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

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


	n=getaddrinfo("194.210.157.206",PORT,&hints,&res);
	if(n!=0){
        printf("0");
        exit(1);
    }

	fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(fd==-1){
        printf("1");
        exit(1);
    }

	n=connect(fd,res->ai_addr,res->ai_addrlen);
	if(n==-1){
        printf("Erro: %d", errno);
        exit(1);
    }

	while (!strcmp(buffer, "exit")){

		scanf("%s", buffer);
		ptr=strcpy(buffer2, buffer);

        nwrite=write(fd, buffer, nleft);
        if(nwrite==-1)/*error*/exit(1);
		/*nleft = 15;
		while(nleft >0){
			nwrite=write(fd, buffer, nleft);
			if(nwrite==-1)/*error exit(1);
			else if (nwrite == 0) break;
			nleft-=nwrite;
			ptr+=nwrite;
		}*/

		memset(buffer, 0, sizeof(char));
        ptr = buffer;
        nread=read(fd,ptr,nleft);
        if(n==-1)/*error*/exit(1);

		/*while(nleft>0){
			nread=read(fd,ptr,nleft);
			if(n==-1)/*error exit(1);
			else if (nread==0)break;
			nleft-=nread;
			ptr+=nread;
		}*/
        write(1, ptr, strlen(ptr));
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(fd);
}