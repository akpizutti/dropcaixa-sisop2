#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>

//  rodar com ./server e rodar client
//  sugestoes do professor
//  main em loop recebendo requests e criando threads para cada accept com cliente(device)
//  switch case em loop dentro da thread que trata comandos que o user enviar
//  thread no cliente fica monitorando modificações de arquivos: inotify via o socket criado para alertar o server
//  -> server entao propaga uma mensagem de alteraçao para todos os devices
//  outra thread no cliente para receber push de arquivos do server
//  logo 3 threads: cuida inotify, recebe push, comandos do user

// estrutura deve ter lista de users conectados no server, cada nodo representa = 1 username, numero de devices conectados do user (max 2), sockets dos devices, descritor das threads criadas

#define PORT 4000
#define MAX_CLIENTS 5
#define MAX_USERNAME_LENGTH 50
#define MAX_DEVICES 2
#define BUFFER_SIZE 1024
#define SYNC_DIR "sync_dir/"

void createSyncDir(){
	const char *folder;
	folder = "./sync_dir";
	struct stat sb;

	if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		printf("Directory exists\n");
	}
	else
	{
		printf("Creating sync_dir...\n");
		mkdir(folder, 0700);
		printf("sync_dir created!\n");
	}
}

void commandMenu()
{
  printf("1. upload <path/filename.ext>\n");
  printf("2. download <filename.ext>\n");
  printf("3. delete <filename.ext> \n");
  printf("4. list_server\n");
  printf("5. list_client \n");
  printf("6. get_sync_dir\n");
  printf("7. exit\n");
}

void receive_and_save_file(int client_socket, const char *file_path) {
    char full_file_path[256];
    snprintf(full_file_path, sizeof(full_file_path), "%s%s", SYNC_DIR, file_path); // salva em sync_dir

    FILE *file = fopen(full_file_path, "wb"); // Open the file for writing in binary mode

    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received < 0) {
        perror("Error receiving file data");

    }

    fclose(file);
}

void process_file_sync_request(char *request, int client_socket) {
    // <action> <path>

    char *action = strtok(request, " "); // Tokenize based on space
    char *path = strtok(NULL, " "); // Get the path

//    switch(action){
//        switch(path){
//            "upload":
//                break;
//            "download":
//                break;
//        }
//        "exit":
//        break;
//        default: break;
//    }


    if (action != NULL) {
        if (path != NULL) {
            if (strcmp(action, "upload") == 0) {
                receive_and_save_file(client_socket, file_path);
            } else if (strcmp(action, "download") == 0) {
                // Handle file download
                // Implement logic to send the requested file to the client from the specified path
            } else if (strcmp(action, "delete") == 0) {
                // Handle file deletion
                // Implement logic to delete the file at the specified path
            } else {
                printf("Invalid command");
            }
        }
        else {
            if (strcmp(action, "get_sync_dir") == 0) {
                createSyncDir();

            } else if (strcmp(action, "list_server") == 0) {

            } else if (strcmp(action, "list_client") == 0) {

            } else if (strcmp(action, "exit")) {
            	bzero(buffer, 256);
            	close(client_socket);
            	pthread_exit(NULL);
            }
            else {
            // todo
            }
        }
    } else {
        printf("Invalid command. <action> <path?>");
    }
}

// Function to handle each client connection
void *handle_client(void *arg)
{
	int client_socket = *((int *)arg);
//	char buffer[1024] = {0};
//	char *i_gotchu_message = "I got your message!\n";
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

    commandMenu();
	// Read data from client
	while (1)
	{
		int message;
		/* read from the socket */
		message = read(client_socket, buffer, sizeof(buffer));
		if (message == 0)
		{
			printf("Client disconnected.\n");
			break;
		}
		else if (message == -1)
		{
			perror("ERROR reading from socket");
			break;
		}
		/* write in the socket */
		else
		{
//			printf("Here is the message: %s\n", buffer);
            process_file_sync_request(buffer, client_socket);
//			message = write(client_socket, i_gotchu_message, strlen(i_gotchu_message));
			if (message < 0)
				printf("ERROR writing to socket");
		}
	}

	// n = read(client_socket, buffer, 256);
	// if (n < 0)
	// 	printf("ERROR reading from socket");
	// printf("Here is the message: %s\n", buffer);

	// n = write(client_socket, "I got your message", 18);
	// if (n < 0)
	// 	printf("ERROR writing to socket");

	// close(client_socket);
	bzero(buffer, 256);
	// Close the client socket
	close(client_socket);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

	// SOCKET LOGIC
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

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding");


	// TCP LISTEN
	listen(sockfd, MAX_CLIENTS);

	while (1)
	{
		printf("Waiting for connections...\n");

		clilen = sizeof(struct sockaddr_in);
		if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
			printf("ERROR on accept");

		printf("New client connected.\n");

		// Create a new thread to handle the client
		pthread_t thread_id;
		if (pthread_create(&thread_id, NULL, handle_client, &newsockfd) != 0)
		{
			perror("pthread_create error");
			//            close(newsockfd);
		}
		printf("Client thread successfully created.\n");

		// bzero(buffer, 256);
	}

	close(sockfd);
	return 0;
}
