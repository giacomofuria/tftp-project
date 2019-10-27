#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> // per la memset
#include <unistd.h> // per la close()
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "costanti.h"

void stampaIndirizzo(struct sockaddr_in str){
	int porta = ntohs(str.sin_port);
	char indirizzo[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &str.sin_addr, indirizzo, INET_ADDRSTRLEN);
	printf("Indirizzo: %s\n",indirizzo);
	printf("Numero di porta: %d\n",porta);
}
void stampa_stringa(char * s, int dim){
	int i;
	printf("Contenuto stringa: ");
	for(i=0; i<dim; i++){
		if(s[i]=='\0'){
			printf("\\0 ");
		}else if(s[i]=='\n'){
			printf("\\n ");
		}else{
			printf("%c ",s[i]);
		}
	}
	printf("\n");
}

struct req_msg{
	uint16_t opcode;
	char filename[MAX_FILENAME_LENGTH];
	char mode[MAX_MODE_LENGTH];
	uint8_t byte_zero;
};

int serialize(struct req_msg* msg, char* buffer){
	int pos = 0;
	uint16_t net_order_opcode = htons(msg->opcode);
	memcpy(buffer+pos, &net_order_opcode, sizeof(msg->opcode));
	pos+=sizeof(msg->opcode);
	
	strcpy(buffer+pos, msg->filename);
	pos+=strlen(msg->filename)+1;
	
	memcpy(buffer+pos, &msg->byte_zero, sizeof(msg->byte_zero));
	pos+=sizeof(msg->byte_zero);
	
	strcpy(buffer+pos, msg->mode);
	pos+=strlen(msg->mode)+1;
	
	memcpy(buffer+pos, &msg->byte_zero, sizeof(msg->byte_zero));
	pos+=sizeof(msg->byte_zero);
	printf("pos=%d\n",pos);
	return pos;
}

void invia(int sd, struct sockaddr_in* sv_addr){
	int ret, len;
	char buf[MAX_BUF_SIZE];
	struct req_msg richiesta;
	richiesta.opcode = RRQ;
	strcpy(richiesta.filename, "giacomo.bin\0");
	strcpy(richiesta.mode, "octet\0");
	
	len = serialize(&richiesta, buf);
	
	ret = sendto(sd, buf, len, 0, (struct sockaddr*)sv_addr, sizeof(*sv_addr));
	if(ret < 0){
		perror("Errore invio richiesta");
		exit(0);
	}else{
		printf("Richiesta inviata correttamente, inviati=%d\n",ret);
	}
}

int main(int argc, char* argv[]){
	int ret, sd;

	struct sockaddr_in my_addr, sv_addr;

	printf("Ci sono %d argomenti\n",argc);
	if(argc >= 3){	
		printf("Indirizzo del server: %s\n",argv[1]);
		printf("Porta del server: %s\n",argv[2]);
	}else{
		printf("Errore! Inserisci tutti i parametri necessari\n");
		exit(0);
	}
	/* Socket con cui il client invia le sue richieste */
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Indirizzo del client */
	memset((void*)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(4243);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(ret < 0){
		perror("Errore nella bind: ");
		exit(0);
	}

	/* Indirizzo del server */
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &sv_addr.sin_addr);

	stampaIndirizzo(sv_addr);
	/*
	uint16_t opcode = htons(1); //RRQ=1
	uint8_t zeri = 0x00;
	
	char buffer[MAX_BUF_SIZE];
	int pos = 0;

	memcpy(buffer+pos, &opcode, sizeof(opcode));
	pos+=sizeof(opcode);
	
	strcpy(buffer+pos, "prova.bin\0");
	pos+=strlen("prova.bin\0")+1;
	
	memcpy(buffer+pos, &zeri, sizeof(zeri));
	pos+=sizeof(zeri);
	
	strcpy(buffer+pos, "octet\0");
	pos+=strlen("octet\0")+1;
	
	memcpy(buffer+pos, &zeri, sizeof(zeri));
	pos+=sizeof(zeri);
	
	len = pos;
	printf("pos=%d\n",pos);
	ret = sendto(sd, buffer, len, 0, (struct sockaddr*)&sv_addr, sizeof(sv_addr));
	if(ret < 0){
		perror("Errore invio richiesta");
		exit(0);
	}else{
		printf("Richiesta inviata correttamente, inviati=%d\n",ret);
	}
	*/
	invia(sd, &sv_addr);
	
	close(sd);

	return 0;
}
