#makefile

CC = gcc
CFLAGS = -Wall

all: client server

client: client.o tftp.o
	$(CC) client.o lib/tftp.o -o client

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

tftp.o: lib/tftp.c
	$(CC) $(CFLAGS) -c lib/tftp.c -o lib/tftp.o

server: server.o tftp.o
	$(CC) server.o lib/tftp.o -o server

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -o server.o
clean:
	rm *.o lib/*.o client server
