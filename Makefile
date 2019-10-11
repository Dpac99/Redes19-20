main_server:
	gcc -o main_server -Wall main_server.c *.h udp_server_handlers.c tcp_server_handlers.c helpers.c
clean:
	rm main_server