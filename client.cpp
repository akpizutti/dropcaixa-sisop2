#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "packet.hpp"

// tamanho máximo de arquivo em bytes

#define PORT 4000
// rodar com ./client localhost
// s

/*
int send_file(int socket, const char* path) {
    char packet_buffer[SIZE_PACKET];
    char file_buffer[MAX_FILE_SIZE];
    FILE * file = fopen(path, "r+");

    if (file == NULL) {printf("File error\n"); close(socket); exit(0);}

    bzero(file_buffer,MAX_FILE_SIZE);
    fgets(file_buffer,MAX_FILE_SIZE, file);


    // cria um pacote com o conteúdo lido do arquivo
    Packet packet = create_packet(1,1,1,1,file_buffer);

    bzero(packet_buffer,SIZE_PACKET);

    serialize_packet(packet, packet_buffer);
    printf("Sending packet:\n");
    print_packet(packet);


    // manda o pacote
    // write in the socket
      int n = write(socket, packet_buffer,SIZE_PACKET);
    bzero(packet_buffer,SIZE_PACKET);
    if (n < 0)
          return -1;

    return 0;

} */

void handle_user_commands(char command, int sockfd)
{

    std::string file_path;
    // Handle different commands
    switch (command)
    {

    case 'l':
        // Handle list files command
        // ...

        break;

    case 'g':
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
        break;

    case 'u':

        std::cout << "Enter the path of file to send: ";

        std::cin >> file_path;
        std::cout << "\npath: " << file_path;

        send_file(file_path.data(), sockfd);

        break;

    default:
        // Handle invalid command
        std::cout << "Invalid command\n";
        break;
    }
}

void print_available_commands()
{
    std::cout << "Available commands:\n";
    std::cout << "- u: upload file\n";
    std::cout << "- f: download file\n";
    std::cout << "- d: delete file\n";
    std::cout << "- s: list_server\n";
    std::cout << "- c: list_client\n";
    std::cout << "- g: get_sync_dir\n";
    std::cout << "- q: quit\n";

    return;
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 2) // mudar pra 3 quando tiver porta p escolher
    {
        std::cerr << "Usage: " << argv[0] << " <username> <server_ip_address> <port>" << std::endl;
        exit(0);
    }

    server = gethostbyname(argv[2]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        printf("ERROR connecting\n");

    // envia username para o server
    Packet id = create_packet(PACKET_USER_ID, 0, 0, 1, argv[1]);
    send_packet(id, sockfd);

    char command = 0;
    while (command != 'q')
    {
        print_available_commands();
        std::cin >> command;
        if (command != 'q')
        {
            handle_user_commands(command, sockfd);
        }
    }

    /* read from the socket */
    // n = read(sockfd, buffer, 256);
    // if (n < 0)
    //     printf("ERROR reading from socket\n");

    // printf("%s\n", buffer);

    std::cout << "Disconnecting..." << std::endl;
    close(sockfd);
    return 0;
}
