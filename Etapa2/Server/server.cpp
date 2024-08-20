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

struct thread_args
{
	int socket;
	std::string username;
};

// Function to handle each client connection
void *handle_client(void *arg)
{
	struct thread_args args = *((struct thread_args *)arg);
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
		if (packet.type==0)
		{
			//connected_users.erase(std::remove(connected_users.begin(), connected_users.end(), username), connected_users.end());
			//tirar device connected
			cout << "Client " << username << " disconnected.\n" ;
			break;
		}
		else if (packet.type == -1)
		{
			perror("ERROR reading from socket");
			break;
		}
		/* write in the socket */
		else
		{

			Packet packet_filesize, packet_temp, packet_file_count, packet_mtime;
			long modify_time;

			switch (packet.type)
			{
			case PACKET_FILE_SIGNAL:
				// save filename from packet.payload here
				filename = packet.payload;
				//std::cout << "Receiving file " << filename << " from user " << username <<std::endl;
				packet_filesize = receive_packet(client_socket);
				if(packet_filesize.type != PACKET_FILE_LENGTH){
					cout << "Wrong packet type received. Expected file length. Received " << packet_filesize.type << endl;
				}
				filesize = bytes_to_int(packet_filesize.payload);

				packet_mtime = receive_packet(client_socket);
				modify_time = bytes_to_long(packet_mtime.payload);


				file_buffer_new = (char*)calloc(filesize, sizeof(char));

				if(receive_file(file_buffer_new, client_socket) != -1){
					save_file(sync_dir_user+"/"+filename, filesize, file_buffer_new);
				}

				

				break;

			case PACKET_GET_SYNC_DIR:
				  
				for (const auto &entry : fs::directory_iterator(sync_dir_user))
				{
					if (entry.is_regular_file())
					{
						//std::cout << entry.path().filename().string() << std::endl;
						files_to_send.push_back(entry.path().filename().string());
						file_count++;
					}
        		}
        
				packet_file_count = create_packet(PACKET_FILE_COUNT,1,1,1,int_to_bytes(file_count));
				send_packet(packet_file_count, client_socket);

				//for(int i=0; i<file_count; i++){}

				for(string fn : files_to_send){
					send_file(sync_dir_user+"/"+fn,client_socket);
				}

				files_to_send.clear();
				
				
				
			break;

			case PACKET_DELETE_FILE:
    			filename = packet.payload;
    			std::cout << "Request to delete file: " << filename << std::endl;

    			// Check if the file exists
    			file_to_delete = sync_dir_user + "/" + filename;
    			if (stat(file_to_delete.c_str(), &sb) == 0) {
        			// File exists, delete it
        			if (remove(file_to_delete.c_str()) == 0) {
            			std::cout << "File " << filename << " deleted successfully." << std::endl;
        			} else {
            			std::cerr << "Error deleting file " << filename << std::endl;
        			}
    			} else {
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
