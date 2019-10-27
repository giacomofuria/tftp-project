#makefile

CC = gcc
CFLAGS = -Wall

all: client server

client: client.o
	$(CC) client.o -o client

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

server: server.o
	$(CC) server.o -o server

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -o server.o
clean:
	rm *.o client server
