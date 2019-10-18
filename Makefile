all: FS user
FS:
	gcc -g -o FS -Wall -std=gnu11 server/* others/*
user:	
	gcc -g -o user -Wall -std=gnu11 client/* others/*
clean:
	rm FS user
