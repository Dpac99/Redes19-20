main_server:
	gcc -o main_server -Wall main_server.c *server*.h udp_server_handlers.c tcp_server_handlers.c helpers.c
main_client:	
	gcc -o main_client -Wall main_client.c client_handlers.h helpers.h consts.h client_handlers.c helpers.c
clean:
	rm main_server
