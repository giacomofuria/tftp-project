#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int porta, ret, sd;
	char directory[MAX_DIR_LENGTH];
	char buffer[MAX_BUF_SIZE];
	uint16_t opcode;
	socklen_t addrlen;
	struct sockaddr_in my_addr, cl_addr;
	pid_t pid;
	/* Gestione porte */
	if(argc == 3){
		porta = atoi(argv[1]);
		
		strcpy(directory, argv[2]);
		//printf("\nServer in ascolto sulla porta: %d\n",porta); // DEBUG
		//printf("Directory: \"%s\"\n\n",directory); //DEBUG
	}else{
		printf("Errore! Inserisci tutti i parametri necessari\n");
		exit(0);
	}

	/* Creazione del socket non bloccante */
	sd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if(sd==-1){
		print_err("creazione socket");
		exit(0);
	}
	printf("\nCreazione socket di ascolto avvenuta con successo.\n");
	
	/* Creazione della struttura dati per l'indirizzo del server'*/
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(porta);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//stampaIndirizzo(my_addr); // DEBUG
	
	/* Eseguo la bind tra il socket e l'indirizzo */
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(ret == -1){
		perror("Errore nella bind: ");
		exit(0);
	}
	addrlen = sizeof(cl_addr);
	while(1){
		// Attende un nuovo messaggio
		void* msg = recv_msg(sd, buffer, (struct sockaddr*)&cl_addr,(socklen_t*)&addrlen, &opcode);
	
		pid = fork();
		if(pid==-1){
			print_err("creazione processo figlio\n");
			exit(-1);
		}
		if(pid == 0){ // processo figlio
			printf("\nCreazione processo con pid=%d\n",getpid());
			/* In base al valore di opcode, il server esegue le diverse operazioni */
			if(opcode == RRQ){
					struct req_msg *richiesta;
					richiesta = (struct req_msg*) msg;
					printf("[pid=%d] Ricevuto un messaggio di richiesta, file: \"%s\"\n",getpid(),richiesta->filename);
					// ricerca del file ed invio del file
					FILE *file_ptr;
					strcat(directory,richiesta->filename);
					int mode;
					if(strcmp(richiesta->mode, "octet")==0){
						file_ptr = fopen(directory, "rb");
						mode = BIN;
					}else{
						file_ptr = fopen(directory, "r");
						mode = TXT;
					}
					if(file_ptr == NULL || file_ptr == 0){
						printf("[pid=%d] File \"%s\" non trovato.\n",getpid(),directory);
						send_error(1,"File not found",sd,&cl_addr);
					}else{
						printf("[pid=%d] File \"%s\" trovato, invio il file.\n",getpid(),directory);
						send_data(file_ptr, mode, sd, &cl_addr);
						printf("[pid=%d] Invio del file %s terminato correttamente.\n",getpid(),richiesta->filename);
						fclose(file_ptr);
					}
					//Re-inizializzo la variabile directory con il percorso della cartella
					strcpy(directory, argv[2]);
			}else if(opcode == ERROR){
					struct err_msg* errore = (struct err_msg*) msg;
					printf("[pid=%d] Ricevuto un messaggio di errore: \"%s\"\n",getpid(),errore->err_msg);
			}else{
				// ricezione messaggio con opcode non valido
				printf("[pid=%d] Ricevuto un messaggio non valido\n",getpid());
				send_error(4,"Illegal TFTP Operation",sd,&cl_addr);
			}
			printf("Terminazione processo con pid=%d\n",getpid()); // DEBUG
			exit(0);
		}
		// Processo padre
	}
	close(sd);

	return 0;
}
