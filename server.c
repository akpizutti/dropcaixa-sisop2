#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//beej nao rodou no lab (erro de comp)
// rodar com ./server e rodar client

// sugestoes do professor
// main em loop recebendo requests e criando threads para cada accept com cliente(device)
// switch case em loop dentro da thread que trata comandos que o user enviar
// thread no cliente fica monitorando modificações de arquivos: inotify via o socket criado para alertar o server
// -> server entao propaga uma mensagem de alteraçao para todos os devices
// outra thread no cliente para receber push de arquivos do server
// logo 3 threads: cuida inotify, recebe push, comandos do user

// estrutura deve ter lista de users conectados no server, cada nodo representa = 1 username, numero de devices conectados do user (max 2), sockets dos devices, descritor das threads criadas

#define PORT 4000

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, n;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding");

	listen(sockfd, 5);

	clilen = sizeof(struct sockaddr_in);
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
		printf("ERROR on accept");

	bzero(buffer, 256);

	/* read from the socket */
	n = read(newsockfd, buffer, 256);
	if (n < 0)
		printf("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);

	/* write in the socket */
	n = write(newsockfd,"I got your message", 18);
	if (n < 0)
		printf("ERROR writing to socket");

	close(newsockfd);
	close(sockfd);
	return 0;
}
