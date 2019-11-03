#include "lib/costanti.h"
#include "lib/tftp.h"

int main(int argc, char* argv[]){
	int ret, sd;
	
	struct sockaddr_in my_addr, sv_addr;

	char cmd[MAX_CMD_LENGTH];
	int mode; // di default imposto il modo di trasferimento testuale
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

	
	while(1){
	
		printf("> ");
		memset(cmd, 0, MAX_CMD_LENGTH);
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
		if(strcmp(cmd,"!help\n")==0){
			show_help();
			continue;
		}
		if(strcmp(cmd,"!quit\n")==0)
			break;
		if(componenti[0] != NULL){
			stampa_stringa(componenti[0],10);
			int cmd_code = get_cmd_code(componenti[0]);
			switch(cmd_code){
				case 1:
					if(i==2 && (strcmp(componenti[1],"bin\n")==0 || strcmp(componenti[1],"txt\n")==0)){
						if(strcmp(componenti[1],"bin\n")==0){
							mode = BIN;
						}else{
							mode = TXT;
						}
						printf("mode impostato a:%s\n",componenti[1]);
					}else{
						print_err("inserisci correttamente il modo di trasferimento {txt|bin}");
					}
					break;
				case 2:
					if(i==3){
						printf("get di %s e salva in %s\n",componenti[1],componenti[2]);
					}else{
						print_err("inserisci i parametri di get correttamente");
					}
					break;
				default: 
					print_err("inserisci un comando valido");	
					break;
			}
		}else{
			print_err("inserisci un comando valido");
		}

	}


	send_request(RRQ, "giacomo.bin","octet", sd, &sv_addr);
	//send_error(1,"File not found",sd,&sv_addr);
	
	close(sd);

	return 0;
}
