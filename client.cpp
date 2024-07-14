#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "packet.hpp"

// tamanho máximo de arquivo em bytes
#define MAX_FILE_SIZE 65536

#define PORT 4000
// rodar com ./client localhost
//s

int send_file(int socket, const char* path) {
    char buffer[256];
    FILE * file = fopen(path, "r+");

    if (file == NULL) {printf("File error\n"); close(socket); exit(0);}

    fgets(buffer,256, file);

    // // DEBUG ZONE 
    // printf("packet size: %d", PACKET_SIZE);
    Packet packet;
    packet.type = -1;
    packet.seqn = -1;
    packet.total_size = -1;
    packet.length = -1;
    printf("buffer strlen in client: %u\n", strlen(buffer));
    strcpy(packet.payload, buffer);


    bzero(buffer,256);
    serialize_packet(packet, buffer);
    exit(0);
    // // DEBUG ZONE END

	/* write in the socket */
	  int n = write(socket, buffer, strlen(buffer));
    bzero(buffer,256);
    if (n < 0)
		  return -1;

    return 0;
    
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char filePath[256];

    
    if (argc < 2) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }

	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

    



	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        printf("ERROR connecting\n");

    printf("Enter the path of file to send: ");
    fgets(filePath, 256, stdin);
    // remove \n no final da string de entrada
    filePath[strcspn(filePath, "\n")] = 0;

    send_file(sockfd,filePath);

	/* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0)
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);

	close(sockfd);
    return 0;
}
