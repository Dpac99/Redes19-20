all:  main_client main_server
main_server:
	gcc -g -o main_server -Wall -std=gnu11 server/* others/*
main_client:	
	gcc -g -o main_client -Wall -std=gnu11 client/* others/*
clean:
	rm main_server main_client
