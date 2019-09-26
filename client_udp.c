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
	int fd, addrlen, /*nleft*/nread, nwrite,n, errcode, size; 
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char *ptr,buffer[128], buffer2[128];


	memset(&hints,0,sizeof hints);
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_DGRAM; // UDP socket
	hints.ai_flags=AI_NUMERICSERV;


	n=getaddrinfo(NULL,PORT,&hints,&res);
	if(n!=0){
        printf("0");
        exit(1);
    }

	fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(fd==-1){
        printf("1");
        exit(1);
    }

	while (1){

		scanf("%s", buffer);
        size=strlen(buffer);
		//ptr=strcpy(buffer2, buffer);

        nwrite=sendto(fd, buffer, size,0, res->ai_addr, res->ai_addrlen);
        if(nwrite==-1){
                printf("2");
                exit(1);
        }
		/*nleft = 15;
		while(nleft >0){
			nwrite=sendto(fd, buffer, size,0, res->ai_addr, res->ai_addrlen);
			if(nwrite==-1){
                printf("2");
                exit(1);
            }
			else if (nwrite == 0) break;
			nleft-=nwrite;
			ptr+=nwrite;
		}*/

		memset(buffer, 0, sizeof(char));
        addrlen=sizeof(addr);

        nread=recvfrom(fd,buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
        if(nread==-1){
                printf("3");
                exit(1);
        }
		/*nleft=15; 
        ptr = buffer;
		while(nleft>0){
			nread=recvfrom(fd,buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
			if(n==-1){
                printf("3");
                exit(1);
            }
			else if (nread==0)break;
			nleft-=nread;
			ptr+=nread;
		}*/

        write(1, "echo: ", 6);
		write(1, buffer, nread);
		write(1, "\n", 1);
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(fd);
}