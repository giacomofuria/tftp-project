#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int porta, ret, sd;
	char directory[MAX_DIR_LENGTH];
	socklen_t addrlen;
	struct sockaddr_in my_addr, cl_addr;
	
	/* Gestione porte */
	if(argc == 3){
		porta = atoi(argv[1]);
		printf("Server in ascolto sulla porta: %d\n",porta);
		strcpy(directory, argv[2]);
		printf("Directory: \"%s\"\n",directory);
	}else{
		printf("Errore! Inserisci tutti i parametri necessari\n");
		exit(0);
	}

	/* Creazione del socket */
	sd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

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
	uint16_t opcode;
	
	addrlen = sizeof(cl_addr);

	void* msg = recv_msg(sd, buffer, (struct sockaddr*)&cl_addr,(socklen_t*)&addrlen, &opcode);

	/* Quando implementerò la versione multi-processo il processo dovrà prendere 
	   una copia di cl_addr per sapere a chi inviare i dati o i messaggi di errore */	
	
	/* In base al valore di opcode, il server deve cambiare comportamento */
	
	if(opcode == RRQ){
			struct req_msg *richiesta;
			richiesta = (struct req_msg*) msg;
			printf("\nRicevuto un nuovo messaggio RRQ %d\n",opcode);
			// ricerca del file ed invio del file
			
	}else if(opcode == ERROR){
			struct err_msg* errore = (struct err_msg*) msg;
			printf("\nRicevuto un messaggio di errore %d\n",opcode);
			// vedere operazioni necessarie
			
	}else{
		/* Qui il server riceve un messaggio con un opcode non valido e deve inviare 
		   un messaggio di errore "Illegal TFTP Operation"
		*/
		
	}

	print_msg(opcode, msg);
	
	close(sd);

	return 0;
}
