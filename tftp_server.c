#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int porta, ret, sd;
	char directory[MAX_DIR_LENGTH];
	socklen_t addrlen;
	struct sockaddr_in my_addr, cl_addr;
	pid_t pid;
	/* Gestione porte */
	if(argc == 3){
		porta = atoi(argv[1]);
		
		strcpy(directory, argv[2]);
		printf("\nServer in ascolto sulla porta: %d\n",porta);
		printf("Directory: \"%s\"\n\n",directory);
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
	// Attende un nuovo messaggio
	while(1){
		void* msg = recv_msg(sd, buffer, (struct sockaddr*)&cl_addr,(socklen_t*)&addrlen, &opcode);

		/* Quando implementerò la versione multi-processo il processo dovrà prendere 
		   una copia di cl_addr per sapere a chi inviare i dati o i messaggi di errore */	
	
		pid = fork();
		if(pid==-1){
			print_err("creazione processo figlio\n");
			exit(-1);
		}
		if(pid == 0){
			// processo figlio
			printf("Processo %d creato.\n",getpid());
			
			/* In base al valore di opcode, il server deve cambiare comportamento (posso mettere uno switch-case) */
			if(opcode == RRQ){
					struct req_msg *richiesta;
					richiesta = (struct req_msg*) msg;
					printf("\nRicevuto un nuovo messaggio RRQ %d\n",opcode);
					// ricerca del file ed invio del file
			
					// simulo che il file non sia presente
					//send_error(1,"File not found",sd,&cl_addr);
					FILE *file_ptr;
					strcat(directory,richiesta->filename);
					printf("dir: %s\n",directory);
					int mode;
					if(strcmp(richiesta->mode, "octet")==0){
						file_ptr = fopen(directory, "rb");
						mode = BIN;
					}else{
						file_ptr = fopen(directory, "r");
						mode = TXT;
					}
					if(file_ptr == NULL || file_ptr == 0){
						printf("File \"%s\" non trovato \n",directory);
						send_error(1,"File not found",sd,&cl_addr);
					}else{
						printf("File \"%s\" trovato. \nInvio il file\n",directory);
						send_data(file_ptr, mode, sd, &cl_addr);
						printf("Invio del file %s terminato correttamente.\n",richiesta->filename);
						fclose(file_ptr);
					}
					strcpy(directory, argv[2]); //Copio in directory il percorso della cartella
			}else if(opcode == ERROR){
					struct err_msg* errore = (struct err_msg*) msg;
					printf("\nRicevuto un messaggio di errore %d\n",opcode);
					print_err_msg(errore);
					// vedere operazioni necessarie
			
			}else{
				// ricezione messaggio con opcode non valido
				printf("\nRicevuto un messaggio non valido\n");
				send_error(4,"Illegal TFTP Operation",sd,&cl_addr);
			}
			printf("Processo %d termina.\n",getpid());
			exit(0);
		}
		// Processo padre
		
	}
	close(sd);

	return 0;
}
