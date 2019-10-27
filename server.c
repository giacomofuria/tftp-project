#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> // per la memset
#include <unistd.h> // per la close()
#include <stdlib.h>
#include <time.h>

#include "lib/costanti.h"

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
	
	uint16_t opcode;
	char filename[MAX_FILENAME_LENGTH];
	char mode[MAX_MODE_LENGTH];
	uint8_t zeri;
	char buffer[MAX_BUF_SIZE];
	int pos=0;
	addrlen = sizeof(cl_addr);

	ret = recvfrom(sd, buffer, MAX_BUF_SIZE, 0, (struct sockaddr*)&cl_addr,&addrlen);
	if(ret < 0){
		perror("Errore ricezione richiesta");
		exit(0);
	}else{
		printf("Richiesta ricevuta correttamente, ricevuti=%d\n",ret);
	}

	memcpy(&opcode, buffer+pos, sizeof(opcode));
	opcode = htons(opcode);
	pos+=sizeof(opcode);
	printf("opcode=%d\n",opcode);
	printf("pos:%d\n",pos);

	strcpy(filename, buffer+pos);
	pos+=strlen(filename)+1;
	printf("filename= %s\n",filename);
	printf("pos:%d\n",pos);


	memcpy(&zeri, buffer+pos, sizeof(zeri));
	pos+=sizeof(zeri);
	printf("zeri= %d\n",zeri);
	printf("pos:%d\n",pos);

	strcpy(mode, buffer+pos);
	pos+=strlen(mode)+1;
	printf("mode=%s\n",mode);
	printf("pos:%d\n",pos);

	memcpy(&zeri, buffer+pos, sizeof(zeri));
	pos+=sizeof(zeri);
	printf("zeri= %d\n",zeri);
	printf("pos:%d\n",pos);

	close(sd);

	return 0;
}
