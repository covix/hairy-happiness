CC	=	gcc

.PHONY: clean


all: server client

server: server.c
	$(CC) server.c -o server.out

client: client.c
	$(CC) client.c -o client.out

clean: 
	echo "clean"
