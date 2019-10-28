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

/* Strutture dati usate per rappresentare i messaggi */
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
struct err_msg{
	uint16_t opcode;
	uint16_t err_num;
	char err_msg[MAX_ERROR_LENGTH];
	uint8_t byte_zero;
};

/* Funzioni di debug */
void stampaIndirizzo(struct sockaddr_in str);
void stampa_stringa(char * s, int dim);

/* Funzioni di debug per la stampa a video dei messaggi */
void print_req_msg(int opcode, struct req_msg* msg);
void print_err_msg(struct err_msg *err);
void print_msg(int opcode, void* data);

/* Funzioni per la serializzazione e deserializzazione delle info da inviare */
int serialize_request(int opcode, struct req_msg *msg, char *buffer);
int serialize_error(struct err_msg *err, char * buffer);
int serialize(int opcode, void *data, char *buffer);

/* Funzioni per l'invio di messaggi */
void send_request(int opcode, char* filename, char* mode, int sd, struct sockaddr_in* sv_addr);
void send_error(uint16_t number, char* message, int sd, struct sockaddr_in* sv_addr);

/* Funzioni per la ricezione dei messaggi */

uint16_t recv_msg(int sd, char* buffer, struct sockaddr * cl_addr, socklen_t* cl_addrlen);
