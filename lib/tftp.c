#include "tftp.h"

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

void print_req_msg(int opcode, struct req_msg* msg){
	printf("opcode=%d | filename=%s | mode=%s",msg->opcode,msg->filename,msg->mode);
}

void print_msg(int opcode, void* data){
	printf("\n| ");
	switch(opcode){
		case RRQ:
			print_req_msg(RRQ, (struct req_msg*)data);
			break;
		case DATA:
			break;
		case ERROR:
			break;
		default:
			break;
	}
	printf(" |\n\n");
}

/* Serializza i campi della struttura req_msq e costruisce il buffer di invio.
   Restituisce la lunghezza (in byte) del buffer da inviare (num di byte da inviare)
*/
int serialize_request(int opcode, struct req_msg *msg, char *buffer){
	int pos=0;
	uint16_t net_order_opcode = htons(msg->opcode);
	memcpy(buffer+pos, &net_order_opcode, sizeof(msg->opcode));
	pos+=sizeof(msg->opcode);
	
	strcpy(buffer+pos, msg->filename);
	pos+=strlen(msg->filename)+1;
	
	memcpy(buffer+pos, &msg->byte_zero, sizeof(msg->byte_zero));
	pos+=sizeof(msg->byte_zero);
	
	strcpy(buffer+pos, msg->mode);
	pos+=strlen(msg->mode)+1;
	
	msg->byte_zero = 0x00;
	memcpy(buffer+pos, &msg->byte_zero, sizeof(msg->byte_zero));
	pos+=sizeof(msg->byte_zero);
	//printf("pos=%d\n",pos); // DEBUG
	return pos;
}

int serialize_error(struct err_msg *err, char * buffer){
	int pos=0;
	uint16_t net_order_opcode = htons(err->opcode);
	memcpy(buffer+pos, &net_order_opcode, sizeof(err->opcode));
	pos+=sizeof(err->opcode);

	uint16_t net_order_number = htons(err->err_num);
	memcpy(buffer+pos, &net_order_number, sizeof(err->err_num));
	pos+=sizeof(err->err_num);

	strcpy(buffer+pos, err->err_msg);
	pos+=strlen(err->err_msg)+1;

	err->byte_zero = 0x00;
	memcpy(buffer+pos, &err->byte_zero, sizeof(err->byte_zero));
	pos+=sizeof(err->byte_zero);
		
	return pos;
}

int serialize(int opcode, void *data, char *buffer){
	int ret = 0;
	
	switch(opcode){
		case RRQ:
			/* è necessario specificare di nuovo opcode perché un msg di 
               richiesta può essere RRQ o WRQ 
            */
			ret = serialize_request(RRQ, (struct req_msg*)data, buffer);
			break;
		case DATA:
			break;
		case ERROR:
			ret = serialize_error((struct err_msg*)data,buffer);
			break;
		default: 
			break;
	}
	return ret;
}

void send_request(int opcode, char* filename, char* mode, int sd, struct sockaddr_in* sv_addr){
	int ret, len;
	char buf[MAX_BUF_SIZE];
	struct req_msg richiesta;
	richiesta.opcode = opcode;
	
	strcat(filename,"\0");
	strcpy(richiesta.filename, filename);
	strcat(mode,"\0");
	strcpy(richiesta.mode, mode);
	
	len = serialize(opcode, (void *)&richiesta, buf);
	
	ret = sendto(sd, buf, len, 0, (struct sockaddr*)sv_addr, sizeof(*sv_addr));
	if(ret < 0){
		perror("Errore invio richiesta");
		exit(0);
	}else{
		printf("Richiesta inviata correttamente, inviati %d byte\n",ret);
	}
	print_msg(RRQ, &richiesta);
}

void send_error(uint16_t number, char* message, int sd, struct sockaddr_in* sv_addr){
	int ret, len;
	char buf[MAX_BUF_SIZE];
	struct err_msg errore;

	errore.opcode = ERROR;
	errore.err_num = number;
	strcat(message, "\0");
	strcpy(errore.err_msg, message);

	len = serialize(ERROR, (void*)&errore, buf);
	
	ret = sendto(sd, buf, len, 0, (struct sockaddr*)sv_addr, sizeof(*sv_addr));
	if(ret < 0){
		perror("Errore invio richiesta");
		exit(0);
	}else{
		printf("Richiesta inviata correttamente, inviati %d byte\n",ret);
	}
	print_msg(ERROR, &errore);
	
}

















