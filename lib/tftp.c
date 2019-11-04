#include "tftp.h"

char *help="Sono disponibili i seguenti comandi:\n";
char *help_help =" !help --> mostra l'elenco dei comandi disponibili\n";
char *help_mode = " !mode {txt|bin} --> imposta il modo di trasferimento dei files (testo o binario)\n";
char *help_get = " !get filename nome_locale --> richiede al server il nome del file <filename> e lo salva localmente con il nome <nome_locale>\n";
char *help_quit = " !quit --> termina il client\n";

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
void print_data_msg(struct data_msg* data){
	printf("\n| ");
	printf("opcode=%d | block number=%d | data=\"%s\"",data->opcode,data->block_number,data->data);
	printf(" |\n\n");
	printf("Byte nel blocco: %d\n",data->num_bytes);
}
void print_req_msg(int opcode, struct req_msg* msg){
	printf("\n| ");
	printf("opcode=%d | filename=\"%s\" | mode=\"%s\"",msg->opcode,msg->filename,msg->mode);
	printf(" |\n\n");
}
void print_err_msg(struct err_msg *err){
	printf("\n| ");
	printf("opcode=%d | number=%d | message=\"%s\"",err->opcode, err->err_num, err->err_msg);
	printf(" |\n\n");
}
void print_msg(int opcode, void* data){
	
	switch(opcode){
		case RRQ:
			print_req_msg(RRQ, (struct req_msg*)data);
			break;
		case DATA:
			print_data_msg((struct data_msg*)data);
			break;
		case ERROR:
			print_err_msg((struct err_msg*)data);
			break;
		default:
			break;
	}
}

int serialize_data(struct data_msg *data,char* buffer){
	int pos = 0;
	uint16_t net_order_tmp;
	net_order_tmp = htons(data->opcode);
	memcpy(buffer+pos, &net_order_tmp, sizeof(data->opcode));
	pos+=sizeof(data->opcode);
	
	net_order_tmp = htons(data->block_number);
	memcpy(buffer+pos, &net_order_tmp, sizeof(data->block_number));
	pos+=sizeof(data->block_number);
	
	memcpy(buffer+pos, &data->data, data->num_bytes);
	pos+=data->num_bytes;
	
	return pos;
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
               richiesta può essere RRQ o WRQ (WRQ non implementato in quanto non richiesto)
            */
			ret = serialize_request(RRQ, (struct req_msg*)data, buffer);
			break;
		case DATA:
			ret = serialize_data((struct data_msg*)data, buffer);
			break;
		case ERROR:
			ret = serialize_error((struct err_msg*)data,buffer);
			break;
		default: 
			break;
	}
	return ret;
}

void deserialize_data(char *buffer, struct data_msg* data){
	int pos=0;
	
	memcpy(&data->opcode, buffer+pos, sizeof(data->opcode));
	data->opcode = ntohs(data->opcode);
	pos+=sizeof(data->opcode);
	
	memcpy(&data->block_number, buffer+pos, sizeof(data->block_number));
	data->block_number = ntohs(data->block_number);
	pos+=sizeof(data->block_number);
	
	/* Copia al max 512 byte, nel caso siano di meno si ferma al terminatore. */
	strncpy(data->data, buffer+pos, BLOCK_SIZE);
	pos+=strlen(data->data)+1;
	
	// aggiungere il valore al campo num_bytes della struttura oppure vedere se è deducibile con strlen
	data->num_bytes=strlen(data->data);
}

void deserialize_request(char* buffer, struct req_msg* req){
	int pos=0;
	memcpy(&req->opcode, buffer+pos, sizeof(req->opcode));
	req->opcode = ntohs(req->opcode);
	pos+=sizeof(req->opcode);

	strcpy(req->filename, buffer+pos);
	pos+=strlen(req->filename)+1;

	memcpy(&req->byte_zero, buffer+pos, sizeof(req->byte_zero));
	pos+=sizeof(req->byte_zero);

	strcpy(req->mode, buffer+pos);
	pos+=strlen(req->mode)+1;

	memcpy(&req->byte_zero, buffer+pos, sizeof(req->byte_zero));
	pos+=sizeof(req->byte_zero);
}

void deserialize_error(char * buffer, struct err_msg* msg){
	int pos=0;
	
	memcpy(&msg->opcode, buffer+pos, sizeof(msg->opcode));
	msg->opcode = ntohs(msg->opcode);
	pos+=sizeof(msg->opcode);

	memcpy(&msg->err_num, buffer+pos, sizeof(msg->err_num));
	msg->err_num = ntohs(msg->err_num);
	pos+=sizeof(msg->err_num);

	strcpy(msg->err_msg, buffer+pos);
	pos+=strlen(msg->err_msg)+1;

	memcpy(&msg->byte_zero, buffer+pos, sizeof(msg->byte_zero));
	pos+=sizeof(msg->byte_zero);

}

void* deserialize(int opcode, char* buffer){
	void *msg;
	switch(opcode){
		case RRQ:
			msg =  malloc(sizeof(struct req_msg));
			deserialize_request(buffer, (struct req_msg*)msg);
			break;
		case DATA:
			msg = malloc(sizeof(struct data_msg));
			deserialize_data(buffer, (struct data_msg*)msg);
			break;
		case ERROR:
			msg = malloc(sizeof(struct err_msg));
			deserialize_error(buffer, (struct err_msg*)msg);
			break;
		default:
			msg = NULL;
			break;
	}
	return msg;
}
/* 	Funzione che legge dal file i dati e invia messaggi di tipo data al client.
	Sono sicuro che il file sia presente e sia stato aperto correttamente
*/
void send_data(FILE *file_ptr, int mode, int sd, struct sockaddr_in* sv_addr){
	int ret, len;
	struct data_msg data;
	char buf[MAX_DATA_SIZE]; // risultato della serialize_data

	data.opcode = DATA;
	data.block_number=0;

	data.num_bytes=0; // contatore dei byte di un blocco e indice all'interno del blocco
	if(mode==TXT){
		while(!feof(file_ptr)){
			data.data[data.num_bytes] = fgetc(file_ptr);
			if(data.data[data.num_bytes] == EOF)
				data.data[data.num_bytes]='\0'; // Quando incontro l'EOF aggiungo un \0 per deserializzare
				//printf("Fine file\n");
			data.num_bytes++;
			if(data.num_bytes == BLOCK_SIZE){
				printf("Blocco %d %d\n",data.block_number, data.num_bytes);
				len = serialize_data(&data, buf);
				ret = sendto(sd, buf, len, 0, (struct sockaddr*)sv_addr, sizeof(*sv_addr));
				if(ret < 0){
					perror("Errore invio blocco");
					exit(0);
				}
				data.num_bytes=0;
				data.block_number++;
			}
		}
		if(data.num_bytes > 0){
			// ultimo blocco con dimensione < 512 byte
			printf("Blocco %d %d byte\n",data.block_number, data.num_bytes);
			len = serialize_data(&data, buf);
			ret = sendto(sd, buf, len, 0, (struct sockaddr*)sv_addr, sizeof(*sv_addr));
			if(ret < 0){
				perror("Errore invio blocco");
				exit(0);
			}
		}
	}else{
		// lettura e invio in modo binario
		
	}
	//printf("File composto da %d byte\n",data.num_bytes);
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
		//printf("Richiesta inviata correttamente, inviati %d byte\n",ret); // DEBUG
	}
	//print_msg(RRQ, &richiesta); // DEBUG
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
		//printf("Richiesta inviata correttamente, inviati %d byte\n",ret);
	}
	//print_msg(ERROR, &errore); // DEBUG
}

/* Riceve un messaggio generico, legge il campo opcode e lo restituisce. Scrivere
   il messaggio ricevuto nel parametro passato buffer */
void* recv_msg(int sd, char* buffer, struct sockaddr * cl_addr, socklen_t* cl_addrlen, uint16_t* opcode){
	void *msg;
	int ret;
	do{
		ret = recvfrom(sd, buffer, MAX_BUF_SIZE, 0, cl_addr,cl_addrlen);
	}while(ret < 0);

	printf("Messaggio ricevuto correttamente, ricevuti %d byte\n",ret); // DEBUG

	memcpy(opcode, buffer, sizeof(*opcode));
	*opcode = ntohs(*opcode);

	msg = deserialize(*opcode, buffer);
	print_msg(*opcode, msg); // DEBUG

	return msg;
}

/* Funzioni per l'interfaccia del client */

void show_help(){
	printf("\n%s %s %s %s %s \n",help,help_help, help_mode,help_get, help_quit);
}

void print_err(char * msg){
	printf("Errore: %s\n",msg);
}
int get_cmd_code(char* cmd){
	if(strcmp(cmd,"!help\n")==0)
		return 0;
	if(strcmp(cmd,"!mode")==0)
		return 1;
	if(strcmp(cmd,"!get")==0)
		return 2;
	if(strcmp(cmd,"!quit\n")==0)
		return 3;
	return -1;
}

















