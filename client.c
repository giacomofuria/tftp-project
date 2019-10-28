#include "lib/costanti.h"
#include "lib/tftp.h"

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
	
	send_request(RRQ, "giacomo.bin","octet", sd, &sv_addr);
	//send_error(1,"messaggio di errore",sd,&sv_addr);
	
	close(sd);

	return 0;
}
