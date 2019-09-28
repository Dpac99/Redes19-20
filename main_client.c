#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>


struct User{
	char userId[5];
	char selected_topic[10];		
	char selected_question[10];
};

void parseArgs(int argc, char *argv[], char *port, char *server_IP);

int main(int argc, char *argv[]){

	
	int tcp_fd, udp_fd, addrlen;
	int nleft, nread, nwrite, n, errcode, size;

	struct addrinfo hints, *res;
	struct sockaddr_in addr;
	char *ptr,buffer[128], buffer2[128], *port, *server_IP;

	port = (char*)malloc(16);
	server_IP = (char*)malloc(128);


	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;      // IPv4
	hints.ai_socktype=SOCK_DGRAM; // UDP socket
	hints.ai_flags=AI_NUMERICSERV;

	parseArgs(argc, argv, port, server_IP);

	if (server_IP != NULL){
		printf("%s\n", server_IP);
	}

	n=getaddrinfo(server_IP, port, &hints, &res);
	if(n!=0){
        printf("0");
        exit(1);
    }

	udp_fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(udp_fd==-1){
        printf("1");
        exit(1);
    }

	// UDP
	while (1){

		scanf("%s", buffer);
        size=strlen(buffer);
		//ptr=strcpy(buffer2, buffer);

        nwrite=sendto(udp_fd, buffer, size, 0, res->ai_addr, res->ai_addrlen);
        if(nwrite==-1){
                printf("2");
                exit(1);
        }
		
		memset(buffer, 0, sizeof(char));
        addrlen=sizeof(addr);

        nread=recvfrom(udp_fd,buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
        if(nread==-1){
                printf("3");
                exit(1);
        }
	
        write(1, "echo: ", 6);
		write(1, buffer, nread);
		write(1, "\n", 1);
		memset(buffer, 0, sizeof(char));
	}
	//TCP
	/* 
	n=getaddrinfo(server_IP, port, &hints, &res);
	if(n!=0) exit(1);

	tcp_fd=socket(res->ai_family,res->ai_socktype, res->ai_protocol);
	if(tcp_fd==-1) exit(1);

	n=connect(tcp_fd,res->ai_addr,res->ai_addrlen);
	if(n==-1) exit(1);

	while (1){

		scanf("%s", buffer);
		ptr=strcpy(buffer2, buffer);

		nleft = 15;
		while(nleft >0){
			nwrite=write(tcp_fd, buffer, nleft);
			if(nwrite==-1) exit(1);
			else if (nwrite == 0) break;
			nleft-=nwrite;
			ptr+=nwrite;
		}

		memset(buffer, 0, sizeof(char));

		nleft=15; ptr = buffer;

		while(nleft>0){
			nread=read(tcp_fd,ptr,nleft);
			if(n==-1) exit(1);
			else if (nread==0)break;
			nleft-=nread;
			ptr+=nread;
		}
		memset(buffer, 0, sizeof(char));
	}

	freeaddrinfo(res);
	close(tcp_fd); */
	return 0;
}

void parseArgs(int argc, char *argv[], char *port, char *server_IP){
	if (argc > 1){
		int opt;
		while((opt = getopt(argc, argv, "n:p:")) != -1) {  
        	switch(opt){  
            case 'n': 
				strcpy(server_IP, argv[2]);
				break;
            case 'p':  
				strcpy(port, argv[4]);
				break;
			} 
        }  
    }
	if(strlen(port)== 0) strcpy(port, "58053");
	return;
}