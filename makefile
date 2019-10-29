#makefile

CC = gcc
CFLAGS = -Wall

all: tftp_client tftp_server

tftp_client: tftp_client.o tftp.o
	$(CC) tftp_client.o lib/tftp.o -o tftp_client

tftp_client.o: tftp_client.c
	$(CC) $(CFLAGS) -c tftp_client.c -o tftp_client.o

tftp.o: lib/tftp.c
	$(CC) $(CFLAGS) -c lib/tftp.c -o lib/tftp.o

tftp_server: tftp_server.o tftp.o
	$(CC) tftp_server.o lib/tftp.o -o tftp_server

tftp_server.o: tftp_server.c
	$(CC) $(CFLAGS) -c tftp_server.c -o tftp_server.o
clean:
	rm *.o lib/*.o tftp_client tftp_server
