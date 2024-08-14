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
#include <filesystem>
#include <sys/inotify.h>

#include "../Common/packet.hpp"

#define PORT 4000
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

void *handle_inotify(void *arg)
{
    int length, i = 0;
    int inotifyfd;
    int inotifywd;
    char buffer[BUF_LEN];
    bool is_watching = true;

    inotifyfd = inotify_init();

    char* username = (char*)arg;
    //std::cout << "Username recebido na handle_inotify: " << username << std::endl;
    std::string sync_dir = "./sync_dir_";
    std::string user_sync_dir = sync_dir + username;


    if (inotifyfd < 0)
    {
        perror("inotify_init");
    }

    while (is_watching == true)
    {

        inotifywd = inotify_add_watch(inotifyfd, user_sync_dir.c_str(),
                                      IN_CLOSE_WRITE | IN_CREATE | IN_DELETE);
        length = read(inotifyfd, buffer, BUF_LEN);

        if (length < 0)
        {
            perror("read");
        }

        i = 0;

        while (i < length)
        {
            struct inotify_event *event =
                (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    printf("The file %s was created.\n", event->name);
                    //send_file()
                }
                else if (event->mask & IN_DELETE)
                {
                    printf("The file %s was deleted.\n", event->name);
                }
                else if (event->mask & IN_CLOSE_WRITE)
                {
                    printf("The file %s was modified.\n", event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    (void)inotify_rm_watch(inotifyfd, inotifywd);
    (void)close(inotifyfd);

    pthread_exit(NULL);
}

void handle_user_commands(char command, int sockfd, std::string username)
{

    std::string file_path;
    std::string sync_dir = "./sync_dir_";
    std::string sync_dir_user;
    sync_dir_user = sync_dir + username;

    Packet packet_sync_signal;

    // Handle different commands
    switch (command)
    {

    case 'l':
        // Handle list files command
        // ...

        break;
        
    case 'c':
        // sincronizar
        namespace fs
        = std::filesystem;
        

        for (const auto &entry : fs::directory_iterator(sync_dir_user))
        {
            if (entry.is_regular_file())
            {
                std::cout << entry.path().filename().string() << std::endl;
            }
        }
        break;

    case 'g':
        struct stat sb;

        //manda sinal pro servidor criar sync_dir
        packet_sync_signal = create_packet(PACKET_GET_SYNC_DIR, 0, 0, 0, NULL);
        send_packet(packet_sync_signal, sockfd);

        if (stat(sync_dir_user.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            printf("Directory exists\n");
        }
        else
        {
            printf("Creating sync_dir...\n");
            mkdir(sync_dir_user.c_str(), 0700);
            
            printf("sync_dir created!\n");
        }
        // inicia sincronização
        break;

    case 'u':
        // sincronizar aqui
        // mutex aqui?
        std::cout << "Enter the path of file to send: ";

        std::cin >> file_path;
        //std::cout << "\npath: " << file_path;

        send_file(file_path.data(), sockfd);
        // quandro server processar e mandar sinal de volta, sincronizar
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

    std::string username = argv[1];

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
    Packet id = create_packet(PACKET_USER_ID, 0, 1, strlen(argv[1]), argv[1]);
    send_packet(id, sockfd);

    // Thread para inotify
    pthread_t inotify_thread;
    if (pthread_create(&inotify_thread, NULL, handle_inotify, argv[1]) != 0)
    {
        perror("pthread_create error");
        exit(1);
    }

    char command = 0;
    print_available_commands();
    while (command != 'q')
    {
        std::cin >> command;
        if (command != 'q')
        {
            handle_user_commands(command, sockfd, username);
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
