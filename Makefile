main_server:
	gcc -o main_server -Wall main_server.c *.h udp_server_handlers.c tcp_server_handlers.c helpers.c

main_client:
	gcc -o main_client -Wall main_client.c *.h client_commands.c client_handlers.c 
clean:
	rm main_server