#include "packet.hpp"
#include "utils.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <algorithm>
#include <math.h>
#include <libgen.h>
#include <iostream>
#include <sys/stat.h>
#include <fstream>

using namespace std;

void print_packet(Packet packet){
    printf("Type: %d\n", packet.type);
    printf("Sequence: %d\n", packet.seqn);
    printf("Total size: %u\n", packet.total_size);
    printf("Length: %d\n", packet.length);
    printf("Payload: \n%s\n", packet.payload);
    return;
}

void print_packet_serialized(char* buffer){
    for (int i =0; i < SIZE_PACKET - MAX_PAYLOAD_SIZE; i++){
        printf("%hhu\n",buffer[i]);
    }
    // printf("Payload: \n");
    // for (int i =PACKET_SIZE - MAX_PAYLOAD_SIZE; i < PACKET_SIZE; i++){
    //     printf("%hhu\n",buffer[i]);
    // }
    return;
}

Packet create_packet(int type, int seqn, int total_size, int length, char* payload){
    Packet ret;
    ret.type = type;
    ret.seqn = seqn;
    ret.total_size = total_size;
    ret.length = length;
    ret.payload = (char*) calloc(MAX_PAYLOAD_SIZE, sizeof(char));
    if(payload != NULL){
        memcpy(ret.payload, payload, MAX_PAYLOAD_SIZE);
    }
    else {
        bzero(ret.payload, MAX_PAYLOAD_SIZE);
    }
    return ret;
}


void serialize_packet(Packet data, char* buffer){
    int offset = 0;

    // sintaxe das operações bitwise é big-endian, mesmo que no hardware seja little-endian
    buffer[offset] = (char)(data.type & 0xff);
    buffer[offset+1] = (char)(data.type >> 8);
    offset += sizeof(data.type);

    buffer[offset] = (char)(data.seqn & 0xff);
    buffer[offset+1] = (char)(data.seqn >> 8);
    offset += sizeof(data.seqn);

    buffer[offset] = (char)(data.total_size & 0xff);
    buffer[offset+1] = (char)((data.total_size >> 8) & 0xff);
    buffer[offset+2] = (char)((data.total_size >> 16) & 0xff);
    buffer[offset+3] = (char)((data.total_size >> 24) & 0xff);
    offset += sizeof(data.total_size);

    buffer[offset] = (char)(data.length & 0xff);
    buffer[offset+1] = (char)(data.length >> 8);
    offset += sizeof(data.length);

    memcpy(buffer+offset, data.payload, data.length);
    offset += MAX_PAYLOAD_SIZE;
    //buffer[offset] = 0; //terminar payload com null (pode ser que precise terminar com outro caractere)

    return;
}



Packet deserialize_packet(char* buffer){
    Packet ret = create_packet(0,0,0,0,0);
    int offset = 0;

    ret.type = ((buffer[0+offset] & 0xff) + (buffer[1+offset] << 8)); 
    offset += sizeof(ret.type);
    ret.seqn = ((buffer[0+offset] & 0xff) + (buffer[1+offset] << 8)); 
    offset += sizeof(ret.seqn);
    ret.total_size = (buffer[0+offset] & 0xff) + ((buffer[1+offset] << 8) & 0xff00) + ((buffer[2+offset] << 16) & 0xff0000) + ((buffer[3+offset] << 24) & 0xff000000); //TODO <-----------------------------
    offset += sizeof(ret.total_size);
    ret.length = ((buffer[0+offset] & 0xff) + (buffer[1+offset] << 8)); 
    offset += sizeof(ret.length);
    memcpy(ret.payload, buffer+offset, ret.length);

    //printf("hi\n");
    return ret;

}

int send_packet(Packet packet, int socket){
    char buffer[SIZE_PACKET];
    int n;
    int bytes_to_write = SIZE_PACKET;
    int offset = 0;

    //print_packet(packet);
    bzero(buffer,SIZE_PACKET);
    serialize_packet(packet, buffer);


    while(bytes_to_write > 0){
        n = write(socket, buffer + offset, SIZE_PACKET - offset);
        bytes_to_write -= n;
        offset += n;
    }

    
    if (n < 0){
        printf("Send error.\n");
        return -1;
    }
    return 0;
}

Packet receive_packet(int socket){
    int n;
    char buffer[SIZE_PACKET];

    int bytes_to_receive = SIZE_PACKET;
    int offset = 0;

    bzero(buffer, SIZE_PACKET);

    while(bytes_to_receive > 0){
        n = read(socket, buffer+offset, SIZE_PACKET - offset);
        bytes_to_receive -= n;
        offset += n;
        if(n==0 || n==-1){
            break;
        }

    }



    if (n == 0)
    {
        printf("Client disconnected.\n");
        return create_packet(0,-1,-1,-1,NULL);
    }
    else if (n == -1)
    {
        perror("ERROR reading from socket");
        return create_packet(-1,-1,-1,-1,NULL);
    }

    return deserialize_packet(buffer);
}


int send_file(std::string file_path, int socket){
    char packet_buffer[SIZE_PACKET];
    char* file_buffer;
    char* filename;
    long int numbytes = 0;
    int bytes_to_read;
    struct stat attrib;

    ifstream file(file_path, ios::in|ios::binary|ios::ate);

    int n;



    if(file.is_open())
    {// obter nome do arquivo
        filename = basename(file_path.data());

        // obter horario de modificação e criação do arquivo
        if(stat(file_path.c_str(),&attrib) == -1)
        {
            cout << "Stat error : " << strerror(errno) << endl;
        }
        time_t modify_time = attrib.st_mtim.tv_sec;
        time_t create_time = attrib.st_ctim.tv_sec;
        
        numbytes = attrib.st_size;



        // enviar packet de signal para recipiente
        Packet signal_packet = create_packet(PACKET_FILE_SIGNAL, 0, 1, strlen(filename), filename);
        send_packet(signal_packet,socket);
        free(signal_packet.payload);


        //enviar packet com tamanho do arquivo
        char *filesize_buffer;
        filesize_buffer = long_to_bytes(numbytes);
        Packet packet_filesize = create_packet(PACKET_FILE_LENGTH,0,1,sizeof(numbytes),filesize_buffer);
        send_packet(packet_filesize,socket);
        free(packet_filesize.payload);

        //enviar packet com timestamp de modificação do arquivo
        Packet packet_mtime = create_packet(PACKET_FILE_MTIME, 1, 1, sizeof(modify_time), long_to_bytes(modify_time));
        send_packet(packet_mtime, socket);
        free(packet_mtime.payload);

        bytes_to_read = numbytes;

        // alocar memória para armazenar conteúdo do arquivo
        file_buffer = new char[numbytes];

        // copia conteúdo do arquivo para a memória
        file.seekg(0,ios::beg);
        file.read (file_buffer, numbytes);

        
        int packets_to_send = ceil((double)bytes_to_read / (double)MAX_PAYLOAD_SIZE);
        int seq = 0;
        int total_size = packets_to_send;

        // manda fragmentos do arquivo se for maior que MAX_PAYLOAD_SIZE
        while(bytes_to_read > MAX_PAYLOAD_SIZE){
            Packet packet = create_packet(PACKET_FILE_DATA,seq,total_size,MAX_PAYLOAD_SIZE,file_buffer+seq*MAX_PAYLOAD_SIZE);

            send_packet(packet,socket);

            seq++;
            bytes_to_read -= MAX_PAYLOAD_SIZE;
            // evitar um memory leak
            free(packet.payload);
        }
        // manda último fragmento se houver
        if(bytes_to_read > 0){
            Packet packet = create_packet(PACKET_FILE_DATA,seq,total_size,bytes_to_read,file_buffer+seq*MAX_PAYLOAD_SIZE);

            send_packet(packet,socket);

            // evitar um memory leak
            free(packet.payload);
        }

        file.close();
        delete[] file_buffer;
    } else{
         cout << "File error. " << endl;
         //send_error(socket);
    }
       
    
    
    return 0;
}


int receive_file(char* buffer, int socket){
    int message;
    char packet_buffer[SIZE_PACKET];
    int bytes_read = 0;

    //receber primeiro pacote


    Packet packet = receive_packet(socket);

    if(packet.type != PACKET_FILE_DATA){
        printf("Wrong packet type received in receive_file.\n");
        std::cout << "Received " << packet.type << ", expected " << PACKET_FILE_DATA << std::endl;
        return -1;
    }

    int total_size = packet.total_size;
    
    memcpy(buffer+bytes_read, packet.payload, packet.length);
    bytes_read += packet.length;
    


    //recebe o resto dos pacotes
    for(int i=1 ; i < total_size; i++){
        Packet packet = receive_packet(socket);
        
        if(packet.type != PACKET_FILE_DATA){
            printf("Wrong packet type received in receive_file.\n");
            return -1;
        }
        memcpy(buffer+bytes_read, packet.payload, packet.length);
        bytes_read += packet.length;
    }

    

    printf("Finished receiving file.\n");
    return bytes_read;
}


void send_error(int socket){
    Packet packet = create_packet(PACKET_ERROR,0,0,0,NULL);
    send_packet(packet,socket);
    return;
}