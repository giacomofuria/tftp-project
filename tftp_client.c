#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int ret, sd;
	int flag = 1;
	struct sockaddr_in my_addr, sv_addr;

	char cmd[MAX_CMD_LENGTH];
	char mode[MAX_MODE_LENGTH];
	strcpy(mode, "octet"); // di default uso il modo di trasferimento binario
	int mode_num = BIN;
	
	char buffer[MAX_BUF_SIZE];
	
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
	
	show_help();
	
	while(flag){
	
		printf("> ");
		memset(cmd, 0, MAX_CMD_LENGTH); // pulizia della stringa contenente il comando
		fgets(cmd, MAX_CMD_LENGTH, stdin); // leggo una riga intera
		char delim[] = " ";
		char *componenti[3];
		int i=0;
		char *ptr = strtok(cmd,delim);
		while(ptr != NULL){
			componenti[i] = ptr; 
			//printf("%s\n",ptr); // DEBUG
			ptr = strtok(NULL,delim);
			i++;
		}
		if(componenti[0] != NULL){
			int cmd_code = get_cmd_code(componenti[0]);
			switch(cmd_code){
				case 0: // comando !help
					show_help();
					break;
				case 1: // comando !mode
					if(i==2 && (strcmp(componenti[1],"bin\n")==0 || strcmp(componenti[1],"txt\n")==0)){
						memset(mode, 0, MAX_MODE_LENGTH); // ripulisco la stringa
						if(strcmp(componenti[1],"bin\n")==0){
							strcpy(mode, "octet");
							mode_num=BIN;
							printf("Modo di trasferimento binario configurato\n");
						}else{
							strcpy(mode, "netascii");
							mode_num=TXT;
							printf("Modo di trasferimento testuale configurato\n");
						}
					}else{
						print_err("inserisci correttamente il modo di trasferimento {txt|bin}");
					}
					break;
				case 2: // comando !get
					if(i==3){
						//printf("get di %s e salva in %s\n",componenti[1],componenti[2]); // DEBUG
						FILE * file_locale;
						int l=strlen(componenti[2])-1;
						componenti[2][l]=0; // tolgo il \n dal fondo del nome del file
						file_locale = fopen(componenti[2],"w");
						if(file_locale == NULL){
							print_err("problema creazione file locale");
							break;
						}
						stampa_stringa(componenti[2],40);
						// invio il messaggio di richiesta al server, in componenti[1] è presente il nome
						send_request(RRQ, componenti[1],mode, sd, &sv_addr);
						printf("Richiesta file %s al server in corso.\n",componenti[1]);
						
						
						/* Riceve i dati e li salva, in caso di risposta negativa 
						   da parte del server non fa nulla. In componenti[2] è presente
						   il nome con cui si vuole salvare il file in locale
						*/
						
						recv_data(sd, buffer, &sv_addr, mode_num, file_locale);
						
						
					}else{
						print_err("inserisci i parametri di get correttamente");
					}
					break;
				case 3: // comando !quit
					flag=0;
					break;
				default: // nessun comando valido
					print_err("inserisci un comando valido");	
					break;
			}
		}else{
			print_err("inserisci un comando valido");
		}

	}
	close(sd);

	return 0;
}
