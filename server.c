#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int porta, ret, sd;
	socklen_t addrlen;
	struct sockaddr_in my_addr, cl_addr;
	
	/* Gestione porte */
	if(argc >= 2){
		porta = atoi(argv[1]);
		printf("Server in ascolto sulla porta: %d\n",porta);
	}else{
		printf("Errore! Inserisci tutti i parametri necessari\n");
		exit(0);
	}

	/* Creazione del socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Creazione della struttura dati per l'indirizzo del server'*/
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(porta);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	stampaIndirizzo(my_addr);
	
	/* Faccio la bind tra il socket e l'indirizzo */
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(ret == -1){
		perror("Errore nella bind: ");
		exit(0);
	}

	char buffer[MAX_BUF_SIZE];
	
	addrlen = sizeof(cl_addr);
	/*
	ret = recvfrom(sd, buffer, MAX_BUF_SIZE, 0, (struct sockaddr*)&cl_addr,&addrlen);
	if(ret < 0){
		perror("Errore ricezione richiesta");
		exit(0);
	}else{
		printf("Richiesta ricevuta correttamente, ricevuti %d byte\n",ret);
	}

	struct req_msg richiesta;
	int pos=0;
	memcpy(&richiesta.opcode, buffer+pos, sizeof(richiesta.opcode));
	richiesta.opcode = htons(richiesta.opcode);
	pos+=sizeof(richiesta.opcode);

	strcpy(richiesta.filename, buffer+pos);
	pos+=strlen(richiesta.filename)+1;


	memcpy(&richiesta.byte_zero, buffer+pos, sizeof(richiesta.byte_zero));
	pos+=sizeof(richiesta.byte_zero);

	strcpy(richiesta.mode, buffer+pos);
	pos+=strlen(richiesta.mode)+1;

	memcpy(&richiesta.byte_zero, buffer+pos, sizeof(richiesta.byte_zero));
	pos+=sizeof(richiesta.byte_zero);

	print_msg(RRQ, &richiesta);
	*/

	uint16_t opcode = recv_msg(sd, buffer, (struct sockaddr*)&cl_addr,(socklen_t*)&addrlen);
	printf("opcode=%d\n",opcode);
	
	struct req_msg richiesta;

	deserialize(opcode, buffer, (void*)&richiesta);
	print_msg(opcode, (void*)&richiesta);

	close(sd);
	
	

	return 0;
}
