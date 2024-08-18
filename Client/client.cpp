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
#include <vector>
#include <mutex>
#include <algorithm>
#include <sys/inotify.h>

#include "../Common/packet.hpp"
#include "../Common/utils.hpp"

#define PORT 4000
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

using namespace std;

namespace fs = std::filesystem;

bool is_watching = false;

struct thread_args
{
    int socket;
    std::string username;
};

vector<string> send_queue;
mutex mutex_send_file;

void *handle_inotify(void *arg)
{
    int length, i = 0;
    int inotifyfd;
    int inotifywd;
    char buffer[BUF_LEN];


    struct thread_args args = *((struct thread_args *)arg);

    int socket = args.socket;

    std::string username = args.username;
    // std::cout << "Username recebido na handle_inotify: " << username << std::endl;
    string user_sync_dir = get_sync_dir_relative_path(username);

    inotifyfd = inotify_init();
    inotifywd = inotify_add_watch(inotifyfd, user_sync_dir.c_str(), IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MOVED_TO);

    if (inotifyfd < 0)
    {
        perror("inotify_init");
    }

    while (is_watching == true)
    {
        sleep(1);
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
                if (event->mask & IN_CREATE || event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO)
                {
                    // printf("The file %s was created.\n", event->name);
                    std::string filename = event->name;

                    sleep(1); // it just works™

                    mutex_send_file.lock();
                    // auto itr = find(send_queue.begin(), send_queue.end(), filename) ;
                    // if (itr == send_queue.end()){
                    //     send_queue.push_back(filename);
                    //     send_file(user_sync_dir + "/" + filename, socket);
                    //     send_queue.erase(itr);
                    // } 
                    send_file(user_sync_dir + "/" + filename, socket);
                    mutex_send_file.unlock();
                        
                }
                else if (event->mask & IN_DELETE)
                {
                    printf("The file %s was deleted.\n", event->name);
                    // TODO: pedir pro servidor deletar arquivo aqui <-----------
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    (void)inotify_rm_watch(inotifyfd, inotifywd);
    (void)close(inotifyfd);

    pthread_exit(NULL);
}

void get_all_files(string username, int socket){
    string sync_dir_user = get_sync_dir_relative_path(username);

    //envia sinal para baixar todos os arquivos
    Packet packet_sync_signal = create_packet(PACKET_GET_SYNC_DIR, 0, 0, 0, NULL);
    send_packet(packet_sync_signal, socket);

    
    Packet packet_filecount = receive_packet(socket);
    
    int filecount = bytes_to_int(packet_filecount.payload);

    is_watching = false;

    for(int i=0; i<filecount; i++){
        Packet packet_signal = receive_packet(socket);
        if(packet_signal.type != PACKET_FILE_SIGNAL){
            cout << "Received wrong packet type in get_all_files: " << packet_signal.type << endl;
            return;
        }

        string filename = packet_signal.payload;

        Packet packet_filesize = receive_packet(socket);

        int filesize = bytes_to_int(packet_filesize.payload);

        Packet packet_mtime = receive_packet(socket);
        long modify_time = bytes_to_long(packet_mtime.payload);

        //char* file_buffer_new = (char*)calloc(filesize, sizeof(char));
        char* file_buffer = new char[filesize];

        
        if(receive_file(file_buffer, socket) != -1){
            save_file(sync_dir_user+"/"+filename, filesize, file_buffer);
        }
    }

    is_watching = true;

    return;

}

void handle_user_commands(char command, int sockfd, std::string username)
{

    std::string file_path;
    string sync_dir_user = get_sync_dir_relative_path(username);

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

        for (const auto &entry : fs::directory_iterator(sync_dir_user))
        {
            if (entry.is_regular_file())
            {
                std::cout << entry.path().filename().string() << std::endl;
            }
        }
        break;

    case 'g':

        // manda sinal pro servidor criar sync_dir


        get_all_files(username, sockfd);

        break;

    case 'u':
        // sincronizar aqui
        // TODO: mutex aqui?
        std::cout << "Enter the path of file to send: ";

        std::cin >> file_path;

        // save_file()
        fs::copy(file_path, sync_dir_user, fs::copy_options::overwrite_existing);

        mutex_send_file.lock();
        send_file(file_path, sockfd);
        mutex_send_file.unlock();
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

    create_sync_dir(username);

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
    struct thread_args args = {sockfd, argv[1]};
    pthread_t inotify_thread;
    is_watching = true;
    if (pthread_create(&inotify_thread, NULL, handle_inotify, &args) != 0)
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
