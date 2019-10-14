main_server:
    gcc -g  -o  main_server -Wall   server/main_server.c    server/*server*.h   server/udp_server_handlers.c    server/tcp_server_handlers.c    others/helpers.c    others/consts.h
main_client:	
    gcc -g  -o  main_client -Wall   client/main_client.c    client/client_commands.h    client/client_handlers.h    others/helpers.h    others/consts.h client/client_commands.c    client/client_handlers.c    others/helpers.c
clean:
    rm main_server main_client                        