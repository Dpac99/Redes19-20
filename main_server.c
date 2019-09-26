#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define PORT "58000"

int max(int x, int y) 
{ 
    if (x > y) 
        return x; 
    else
        return y; 
} 

int main(){
    struct sigaction act;
	struct addrinfo hints, *res, *i;
    struct sockaddr_in addr;
    pid_t pid;
    socklen_t addrlen;
    int udp_fd = 0 ,tcp_fd = 0, errcode, maxfd, nready, resp_fd;
    ssize_t n, nread, nsent;
    char buffer[128], buffer2[128], *ptr;
    fd_set rfds;

    act.sa_handler=SIG_IGN;
    if(sigaction(SIGCHLD,&act,NULL)==-1){
        printf("Error with sigaction\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0; // Accepts TCP and UDP sockets
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if ((errcode = getaddrinfo(NULL, PORT, &hints, &res)) != 0){
        printf("Error with getaddrinfo\n");
        exit(1);
    }

    for(i=res; i!=NULL; i=i->ai_next){
        if (i->ai_socktype == SOCK_DGRAM){ //UDP Socket 
    
            if((udp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1){
                if(tcp_fd > 0){
                    close(tcp_fd);
                }
                freeaddrinfo(i);
                printf("Error with socket udp\n");
                exit(1);
            }

            if (bind(udp_fd, i->ai_addr, i->ai_addrlen) == -1){
                if(tcp_fd > 0){
                    close(tcp_fd);
                }
                freeaddrinfo(i);
                close(udp_fd);
                printf("Error with bind udp\n");
                exit(1);
            }

        }
        else if (i->ai_socktype == SOCK_STREAM){

            if((tcp_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1){
                 if(udp_fd > 0){
                    close(udp_fd);
                }
                freeaddrinfo(i);
                printf("Error with socket tcp\n");
                exit(1);
            }

            if (bind(tcp_fd, i->ai_addr, i->ai_addrlen) == -1){
                freeaddrinfo(i);
                close(udp_fd);
                if(tcp_fd > 0){
                    close(udp_fd);
                }
                printf("Error with bind tcp\n");
                exit(1);
            }

            if (listen(tcp_fd, SOMAXCONN) == -1){
                if(udp_fd > 0)
                    close(udp_fd);
                freeaddrinfo(i);
                close(tcp_fd);
                printf("Error with listen tcp\n");
                exit(1);
            }
        }
    }
    maxfd = max(tcp_fd, udp_fd) + 1; 
    freeaddrinfo(res);

    while(1){
        FD_ZERO(&rfds); 
        FD_SET(tcp_fd, &rfds);
        FD_SET(udp_fd, &rfds);

        nready=select(maxfd, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval*)NULL);
        if (nready < 0){
            close(tcp_fd);
            close(udp_fd);
            printf("Error with select\n");
            exit(1);
        }


        if(FD_ISSET(udp_fd, &rfds)){
            memset(buffer, 0, sizeof(char));
            memset(buffer2, 0, sizeof(char));
            n, nsent, nread = 0;
            addrlen = sizeof(addr);
            nread = recvfrom(udp_fd, buffer, 128, 0, (struct sockaddr*)&addr, &addrlen);

            if(nread == -1){
                close(tcp_fd);
                close(udp_fd);
                printf("Error with recvfrom\n");
                exit(1);
            }

            printf("Received: %s\n", buffer);

            nsent = sendto(udp_fd, buffer, nread, 0, (struct sockaddr*)&addr, addrlen);

            if(nsent == -1){
                close(tcp_fd);
                close(udp_fd);
                printf("Error with sendto: %d\n", errno);
                exit(1);
            }
        }

        if(FD_ISSET(tcp_fd, &rfds)){
            memset(buffer, 0, sizeof(char));
            memset(buffer2, 0, sizeof(char));
            n, nsent, nread = 0;
            addrlen = sizeof(addr);
            resp_fd = accept(tcp_fd, (struct sockaddr*)&addr, &addrlen);
            if (resp_fd == -1){
                close(tcp_fd);
                close(udp_fd);
                printf("Error with accept tcp\n");
                exit(1);
            }

            if ((pid = fork()) == -1){
                close(tcp_fd);
                close(udp_fd);
                printf("Error with fork\n");
                exit(1);
            }
            else if (pid == 0){ //Child process
                close(tcp_fd);
                while((n=read(resp_fd,buffer2,128))!=0){
                    if(n==-1){
                        close(resp_fd);
                        close(udp_fd);
                        printf("Error with read\n");
                        exit(1);
                    }

                    printf("Received: %s\n", buffer2);

                    ptr=&buffer2[0];
                    while (n>0){
                        if((nsent=write(resp_fd,ptr,n))<=0){
                            close(resp_fd);
                            close(udp_fd);
                            printf("Error with write\n");
                            exit(1);
                        }
                        n-=nsent; 
                        ptr+=nsent;
                    }
                }
            }
        }
    }

    close(resp_fd);
    close(udp_fd);
    return 0;
}