#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> // per la memset
#include <unistd.h> // per la close()
#include <stdlib.h>
#include <errno.h>
#include "costanti.h"
struct req_msg{
	uint16_t opcode;
	char filename[MAX_FILENAME_LENGTH];
	char mode[MAX_MODE_LENGTH];
	uint8_t byte_zero;
};
struct data_msg{
	uint16_t opcode;
	uint16_t block_number;
	char data[BLOCK_SIZE];
};
struct error_msg{
	uint16_t opcode;
	uint16_t error_number;
	uint16_t error_message;
	uint8_t byte_zero;
};

void stampaIndirizzo(struct sockaddr_in str);
void stampa_stringa(char * s, int dim);

void print_req_msg(int opcode, struct req_msg* msg);
void print_msg(int opcode, void* data);

int serialize_request(int opcode, struct req_msg *msg, char *buffer);
int serialize(int opcode, void *data, char *buffer);
void send_request(int opcode, char* filename, char* mode, int sd, struct sockaddr_in* sv_addr);
