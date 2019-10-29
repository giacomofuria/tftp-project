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

	uint16_t opcode = recv_msg(sd, buffer, (struct sockaddr*)&cl_addr,(socklen_t*)&addrlen);

	/* Quando implementerò la versione multi-processo il processo dovrà prendere 
	   una copia di cl_addr per sapere a chi inviare i dati o i messaggi di errore */	

	printf("opcode=%d\n",opcode);
	
	void *msg;

	msg = deserialize(opcode, buffer);
	
	print_msg(opcode, msg);

	close(sd);

	return 0;
}
