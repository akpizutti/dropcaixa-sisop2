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

bool is_connected = false;
bool is_watching = false;

vector<string> send_queue;
mutex mutex_send_file;

connection_info dados_conexao = connection_info();

void get_all_files(string username, int socket){
    string sync_dir_user = get_sync_dir_relative_path(username);

    

    
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

void *handle_inotify(void *arg)
{
    int length, i = 0;
    int inotifyfd;
    int inotifywd;
    char buffer[BUF_LEN];


    //struct connection_info args = *((struct connection_info *)arg);

    //int socket = dados_conexao.socket;

    //std::string username = args.username;
    // std::cout << "Username recebido na handle_inotify: " << username << std::endl;
    string user_sync_dir = get_sync_dir_relative_path(dados_conexao.username);

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
                    send_file(user_sync_dir + "/" + filename, dados_conexao.socket);
                    mutex_send_file.unlock();
                        
                }
                else if (event->mask & IN_DELETE)
                {
                    printf("The file %s was deleted.\n", event->name);
                    string filename = event->name;

                    Packet packet_delete_signal = create_packet(PACKET_DELETE_FILE, 0, 0, filename.size(), (char*)filename.c_str());
                    send_packet(packet_delete_signal, dados_conexao.socket);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    (void)inotify_rm_watch(inotifyfd, inotifywd);
    (void)close(inotifyfd);

    pthread_exit(NULL);
}

void *listenThread(void *arg){
    //struct connection_info args = *((struct connection_info *)arg);
    //int socket = args.socket;
    //std::string username = args.username;
    string user_sync_dir = get_sync_dir_relative_path(dados_conexao.username);

    Packet received_packet;

    cout << "Hello from listenThread\n";

    do{
        received_packet = receive_packet(dados_conexao.socket);

        cout << "listenThread recebeu pacote tipo " << received_packet.type << endl;

        if(received_packet.type!= 0 && received_packet.type !=-1)
        {
            switch(received_packet.type){
            case PACKET_SIGNAL_SYNC:
                get_all_files(dados_conexao.username,dados_conexao.socket);
                break;
            case PACKET_REJECT:
                cout << "Connection rejected.";
                close(dados_conexao.socket);
                exit(0);
                break;

            default:
                cout << "listenThread recebeu pacote errado: " << received_packet.type;
                break;
            }
        }
        
    } while (received_packet.type!= 0 && received_packet.type !=-1);

    cout<< "saindo da listenThread\n";
    pthread_exit(NULL);
}



void handle_user_commands(char command, int sockfd, std::string username)
{

    std::string file_path;
    string sync_dir_user = get_sync_dir_relative_path(username);
    std::string file_to_delete, filename;

    Packet packet_sync_signal;

    // Handle different commands
    switch (command)
    {

    case 'l':
        // Handle list files command
        // ...

        break;
    
    case 'd':
        std::cout << "Enter the name of the file to delete: ";
        std::cin >> filename; 
        

        // Deleta o arquivo localmente no cliente
        file_to_delete = sync_dir_user + "/" + filename;
        if (remove(file_to_delete.c_str()) == 0) {
            std::cout << "File " << filename << " deleted successfully from client." << std::endl;
        } else {
            std::cerr << "Error deleting file " << filename << " from client." << std::endl;
        }

        // thread do inotify manda sinal pro servidor deletar 
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

        //envia sinal para baixar todos os arquivos
        packet_sync_signal = create_packet(PACKET_GET_SYNC_DIR, 0, 0, 0, NULL);
        send_packet(packet_sync_signal, sockfd);

        // listenThread vai receber os arquivos


        //get_all_files(username, sockfd);

        break;

    case 'u':
        std::cout << "Enter the path of file to upload: ";

        std::cin >> file_path;

        // save_file()
        fs::copy(file_path, sync_dir_user, fs::copy_options::overwrite_existing);

        // mutex_send_file.lock();
        // send_file(file_path, sockfd);
        // mutex_send_file.unlock();
        // quandro server processar e mandar sinal de volta, sincronizar
        break;

    case 'f':
        cout << "Enter the name of file to download: ";
        cin >> filename;
        fs::copy(sync_dir_user+"/"+ filename, "./",  fs::copy_options::overwrite_existing);
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

    if (argc < 4) 
    {
        std::cerr << "Usage: " << argv[0] << " <username> <server_ip_address> <port>" << std::endl;
        exit(0);
    }

    std::string username = argv[1];

    int port = atoi(argv[3]);


    

    server = gethostbyname(argv[2]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR opening socket\n");
        exit(0);
    }
        

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR connecting\n");
        exit(0);
    }
        
    is_connected = true;
    cout << "Connected.\n";
    create_sync_dir(username);


    dados_conexao.id = 0;
    dados_conexao.socket = sockfd;
    dados_conexao.username = username;


    // Thread para receber respostas do servidor
    //struct connection_info args_listen = {0, sockfd, argv[1]};
    pthread_t listen_thread;
    if (pthread_create(&listen_thread, NULL, listenThread, NULL) != 0){
        perror("pthread_create error");
        exit(1);
    }

    // Thread para inotify
    //struct connection_info args = {0, sockfd, argv[1]};
    pthread_t inotify_thread;
    is_watching = true;
    if (pthread_create(&inotify_thread, NULL, handle_inotify, NULL) != 0)
    {
        perror("pthread_create error");
        exit(1);
    }

    // envia username para o server
    Packet id = create_packet(PACKET_USER_ID, 0, 1, strlen(argv[1]), argv[1]);
    send_packet(id, sockfd);

    char command = 0;
    print_available_commands();
    while (command != 'q' && is_connected == true)
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
