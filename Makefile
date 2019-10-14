main_server:
	gcc -g -o main_server -Wall server/* others/*
main_client:	
	gcc -g -o main_client -Wall client/* others/*
clean:
	rm main_server
