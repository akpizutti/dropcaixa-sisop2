#include "../Common/users.hpp"
#include "../Common/packet.hpp"
#include "../Common/utils.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <dirent.h>
#include <filesystem>

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

using namespace std;

namespace fs = std::filesystem;

vector<User *> connected_users;
vector<connection_info> client_connections;
vector<connection_info> backups;


bool is_backup = false;

int id = 0;


// Function to handle each client connection
void *handle_client(void *arg)
{
	struct connection_info args = *((struct connection_info *)arg);
	int client_socket = args.socket;

	Packet packet;

	std::string username = args.username;
	string sync_dir_user = get_sync_dir_relative_path(username);

	create_sync_dir(username);

	char file_buffer[MAX_FILE_SIZE];
	char *file_buffer_new;
	vector<string> files_to_send;

	// Read data from client
	while (1)
	{
		int message;
		int filesize = 0;
		std::string filename;
		std::string file_to_delete;
		struct stat sb;
		int file_count = 0;

		/* read from the socket */
		packet = receive_packet(client_socket);
		if (packet.type == 0)
		{
			// connected_users.erase(std::remove(connected_users.begin(), connected_users.end(), username), connected_users.end());
			// tirar device connected
			cout << "Client " << username << " disconnected.\n";


			remove_user(username, &connected_users);
			close(client_socket);
			pthread_exit(NULL);
			break;
		}
		else if (packet.type == -1)
		{
			perror("ERROR reading from socket");
			close(client_socket);
			break;
		}
		/* write in the socket */
		else
		{

			Packet packet_filesize, packet_temp, packet_file_count, packet_mtime, packet_signal_allfiles;
			long modify_time;

			switch (packet.type)
			{
			case PACKET_FILE_SIGNAL:
				// save filename from packet.payload here
				filename = packet.payload;
				// std::cout << "Receiving file " << filename << " from user " << username <<std::endl;
				packet_filesize = receive_packet(client_socket);
				if (packet_filesize.type != PACKET_FILE_LENGTH)
				{
					cout << "Wrong packet type received. Expected file length. Received " << packet_filesize.type << endl;
				}
				filesize = bytes_to_int(packet_filesize.payload);

				packet_mtime = receive_packet(client_socket);
				modify_time = bytes_to_long(packet_mtime.payload);

				file_buffer_new = (char *)calloc(filesize, sizeof(char));

				if (receive_file(file_buffer_new, client_socket) != -1)
				{
					save_file(sync_dir_user + "/" + filename, filesize, file_buffer_new);
				}

				break;

			case PACKET_GET_SYNC_DIR:

				for (const auto &entry : fs::directory_iterator(sync_dir_user))
				{
					if (entry.is_regular_file())
					{
						// std::cout << entry.path().filename().string() << std::endl;
						files_to_send.push_back(entry.path().filename().string());
						file_count++;
					}
				}

				packet_signal_allfiles = create_packet(PACKET_SIGNAL_SYNC, 0, 0, 0, NULL);
				send_packet(packet_signal_allfiles, client_socket);

				packet_file_count = create_packet(PACKET_FILE_COUNT, 1, 1, 1, int_to_bytes(file_count));
				send_packet(packet_file_count, client_socket);

				// for(int i=0; i<file_count; i++){}

				for (string fn : files_to_send)
				{
					send_file(sync_dir_user + "/" + fn, client_socket);
				}

				files_to_send.clear();

				break;

			case PACKET_DELETE_FILE:
				filename = packet.payload;
				std::cout << "Request to delete file: " << filename << std::endl;

				// Check if the file exists
				file_to_delete = sync_dir_user + "/" + filename;
				if (stat(file_to_delete.c_str(), &sb) == 0)
				{
					// File exists, delete it
					if (remove(file_to_delete.c_str()) == 0)
					{
						std::cout << "File " << filename << " deleted successfully." << std::endl;
					}
					else
					{
						std::cerr << "Error deleting file " << filename << std::endl;
					}
				}
				else
				{
					std::cerr << "File " << filename << " does not exist." << std::endl;
				}
				break;

			default:
				printf("Received invalid packet type: %d\n", packet.type);
			}
		}
	}

	// Close the client socket
	close(client_socket);
	pthread_exit(NULL);
}


void *handle_pings(void *arg){
	struct connection_info args = *((struct connection_info *)arg);
	int server_socket = args.socket;

	cout << "Hello from handle_backup\n";

	Packet packet_from_server, packet_to_send;


	while(1){
		packet_from_server = receive_packet(server_socket);
		switch(packet_from_server.type){
			case PACKET_PING_REQUEST:
				cout << "Tô vivo!\n";
				packet_to_send = create_packet(PACKET_PING_REPLY);
				send_packet(packet_to_send,server_socket);
				break;
		}

	}
	

	pthread_exit(NULL);

}

void *ping_thread(void *arg) {
	struct connection_info args = *((struct connection_info *)arg);
	int server_socket = args.socket;

	Packet ping_packet = create_packet(PACKET_PING_REQUEST, 0,0,0,NULL);
	Packet reply_packet;

	while(1){
		sleep(5);
		send_packet(ping_packet,server_socket);
		cout << "Perguntando pro primário se tá vivo...";
		reply_packet = receive_packet(server_socket);
		if(reply_packet.type == PACKET_PING_REPLY){
			cout << "Tá vivo!\n";
		} else {
			cout << "Morreu! \n";
		}
	}

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
	if (argc < 2 || (argc > 2 && argc < 5)) 
    {
        //std::cerr << "Usage: \nPrimary: " << argv[0] << " <port> <backup> <primary_ip> <primary_port>" << std::endl;
		cout << "Usage:\n";
		cout << "Primary: " << argv[0] << " <port>\n";
		cout << "Backup:  " << argv[0] << " <port> <backup> <primary_ip> <primary_port>\n";
        exit(0);
	}

	int port = atoi(argv[1]);

	// SOCKET LOGIC
	int sockfd, newsockfd, n;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr, primary_addr;

	// inicia como backup se receber argumentos de backup
	if(argc > 2 && strcmp(argv[2],"backup") == 0){
		is_backup = true;
		int primary_port = atoi(argv[4]);
		struct hostent *primary = gethostbyname(argv[3]);
		if (primary == NULL)
		{
			fprintf(stderr, "ERROR, no such host\n");
			exit(0);
		}

		cout << "Backup mode.\n";
		cout << "This server's port is " << port <<endl;
		cout << "Primary server's port is " << primary_port <<endl;

		// aqui o backup conecta ao server principal

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
			printf("ERROR opening socket\n");
			exit(0);
		}
		primary_addr.sin_family = AF_INET;
		primary_addr.sin_port = htons(primary_port);
		primary_addr.sin_addr = *((struct in_addr *)primary->h_addr);
		bzero(&(primary_addr.sin_zero), 8);

		if (connect(sockfd, (struct sockaddr *)&primary_addr, sizeof(primary_addr)) < 0){
			printf("ERROR connecting\n");
			exit(0);
		}

		cout << "Connected to primary server.\n";

		string id_backup = "backup";

		Packet id = create_packet(PACKET_USER_ID, 0, 1, id_backup.size(), id_backup.data());
    	send_packet(id, sockfd);

		connection_info primary_info = connection_info();
		primary_info.id = 0;
		primary_info.socket = sockfd;

		pthread_t thread_id;
		if (pthread_create(&thread_id, NULL, ping_thread, &primary_info) != 0)
		{
			perror("pthread_create error");
			//            close(newsockfd);
		}
		printf("ping thread successfully created.\n");

		while(1) {
			continue;
		}
		//exit(0);
	} 
	// a partir daqui só deve executar se for o primário
	else {
		

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			printf("ERROR opening socket");

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);
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
				//std::cout << "Maximum number of clients reached" << std::endl;
				//send(sockfd, "Maximum number of clients reached", 30, 0);
				//close(sockfd);
				//Packet packet_reject = create_packet(PACKET_REJECT, 0,0,0,NULL);
				//send_packet(packet_reject, sockfd);

				do{
					sleep(1);
				} while (connected_users.size() >= MAX_CLIENTS);
				
			}
			else
			{
				clilen = sizeof(struct sockaddr_in);
				if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
					printf("ERROR on accept");

				cout << "Accepting connection. \n";

				Packet user_packet;
				user_packet = receive_packet(newsockfd);

				if (user_packet.type != PACKET_USER_ID)
				{
					std::cout << "Invalid packet type received. Expected user ID\n";
					close(newsockfd);
				} else if (strcmp(user_packet.payload,"backup") == 0){ //testa se foi um backup que conectou
					// fazer coisas se quem conectou foi server de backup
					connection_info backup_info = connection_info();
					backup_info.id = id++;
					backup_info.socket = newsockfd;
					backup_info.username = user_packet.payload;
					backup_info.is_backup = true;

					backups.push_back(backup_info);

					// Cria nova thread para escutar pings do backup
					pthread_t thread_id;
					if (pthread_create(&thread_id, NULL, handle_pings, &backup_info) != 0)
					{
						perror("pthread_create error");
						//            close(newsockfd);
					}
					printf("backup thread successfully created.\n");

				}
				else
				{
					connection_info client_info = connection_info();
					client_info.id = id++;
					client_info.socket = newsockfd;
					client_info.username = user_packet.payload;

					client_connections.push_back(client_info);

					User *new_user = new User(user_packet.payload);
					new_user->set_connected_devices(1);
					// TODO: não deixar conectar mais de 2 clientes por user

					connected_users.push_back(new_user);
					std::cout << "New client connected: " << user_packet.payload << std::endl;

					print_users(connected_users);

					// Create a new thread to handle the client
					pthread_t thread_id;
					if (pthread_create(&thread_id, NULL, handle_client, &client_info) != 0)
					{
						perror("pthread_create error");
						//            close(newsockfd);
					}
					printf("Client thread successfully created.\n");
				}
			}

			// bzero(buffer, 256);
		}
		}

	close(sockfd);
	return 0;
}
