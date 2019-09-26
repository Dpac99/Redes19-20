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
	int fd, addrlen, n, errcode, newfd; 
	struct addrinfo hints,*res;
	struct sockaddr_in addr;
	char buffer[128], host[NI_MAXHOST], service[NI_MAXSERV], buffer2[128];

	memset(&hints,0,sizeof hints);
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_STREAM; // UDP socket
	hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;

	n=getaddrinfo(NULL,PORT,&hints,&res);
	if(n!=0){
        printf("Error here 1");
        exit(1);
    }

	fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(fd==-1){
        printf("Error here 2");
        exit(1);
    }

	n=bind(fd,res->ai_addr,res->ai_addrlen);
	if(n==-1){
        printf("Error here 3");
        exit(1);
    }


    if(listen(fd,5)==-1){
        printf("Error here 4");
        exit(1);
    }

    if((newfd=accept(fd,(struct sockaddr*)&addr, &addrlen))==-1){
        printf("Error here 5");
        exit(1);
    }

	while (1){

		n=read(newfd,buffer,128);
        if(n==-1){
            printf("Error here 6");
            exit(1);
        }
        
        write(1,"received: ",10);
        write(1,buffer,n);
        write(1,"\n",1);

       /* if ((errcode=getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof host, service, sizeof service, 0))!= 0)
			fprintf(stderr, "error: getnameinfo: %s\n",gai_strerror(errcode));
		else
			printf(" from [%s:%s]\n", host, service);
*/
		scanf("%s", buffer2);

        n=write(newfd,buffer2,strlen(buffer2));
        if(n==-1){
            printf("Error here 7");
            exit(1);
        }

        memset(buffer2, 0, sizeof(char));
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(fd);
    close(newfd);

	return 0;
}