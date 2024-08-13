#include "users.hpp"
#include "packet.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>

//  sugestoes do professor
//  main em loop recebendo requests e criando threads para cada accept com cliente(device) DONE
//  switch case em loop dentro da thread que trata comandos que o user enviar
//  thread no cliente fica monitorando modificações de arquivos: inotify via o socket criado para alertar o server
//  -> server entao propaga uma mensagem de alteraçao para todos os devices
//  outra thread no cliente para receber push de arquivos do server
//  logo 3 threads: cuida inotify, recebe push, comandos do user

// estrutura deve ter lista de users conectados no server, cada nodo representa = 1 username, numero de devices conectados do user (max 2), sockets dos devices, descritor das threads criadas

#define PORT 4000
#define MAX_CLIENTS 5

struct thread_args
{
	int socket;
	std::string username;
};

// Function to handle each client connection
void *handle_client(void *arg)
{
	struct thread_args args = *((struct thread_args *)arg);
	// int client_socket = *((int *)arg);
	int client_socket = args.socket;

	std::string username = args.username;

	std::cout << "Username recebido na handle_client: " << username << std::endl;

	char buffer[1024] = {0};
	char file_buffer[MAX_FILE_SIZE];
	char *i_gotchu_message = "I got your message!\n";

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
			printf("Here is the message: %s\n", buffer);
			Packet packet = deserialize_packet(buffer);
			// printf("Received packet:\n");
			// print_packet(packet);

			switch (packet.type)
			{
			case PACKET_FILE_SIGNAL:
				// save filename from packet.payload here
				receive_file(file_buffer, client_socket);

				printf("File received: \n%s\n", file_buffer);
				break;
			default:
				printf("Received invalid packet type.\n");
			}

			message = write(client_socket, i_gotchu_message, strlen(i_gotchu_message));
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

void print_users(std::vector<User *> users_list)
{
	std::cout << "Connected Users:" << std::endl;
	for (int i = 0; i < users_list.size(); i++)
	{
		std::cout << users_list[i]->get_username() << std::endl;
	}
}

int main(int argc, char *argv[])
{
	User *test_user = new User("Paulo");

	// SOCKET LOGIC
	int sockfd, newsockfd, n;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;

	std::vector<User *> connected_users;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printf("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding");

	// SYNCING SERVER AND DEVICES LOGIC
	// TCP LISTEN
	listen(sockfd, MAX_CLIENTS);

	while (1)
	{
		printf("Waiting for connections...\n");

		if (connected_users.size() >= MAX_CLIENTS)
		{
			std::cout << "Maximum number of clients reached" << std::endl;
			send(sockfd, "Maximum number of clients reached", 30, 0);
			close(sockfd);
		}
		else
		{
			clilen = sizeof(struct sockaddr_in);
			if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
				printf("ERROR on accept");

			Packet user_packet;
			user_packet = receive_packet(newsockfd);
			if (user_packet.type != PACKET_USER_ID)
			{
				std::cout << "Invalid packet type received. Expected user ID\n";
				close(newsockfd);
			}
			else
			{

				User *new_user = new User(user_packet.payload);
				new_user->set_connected_devices(1);
				// TODO: não deixar conectar mais de 2 clientes por user

				connected_users.push_back(new_user);
				std::cout << "New client connected: " << user_packet.payload << std::endl;

				print_users(connected_users);

				// Create a new thread to handle the client
				struct thread_args args = {newsockfd, new_user->get_username()};
				pthread_t thread_id;
				if (pthread_create(&thread_id, NULL, handle_client, &args) != 0)
				{
					perror("pthread_create error");
					//            close(newsockfd);
				}
				printf("Client thread successfully created.\n");
			}
		}

		// bzero(buffer, 256);
	}

	close(sockfd);
	return 0;
}
