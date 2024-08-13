#include "packet.hpp"
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
    ret.payload = (char*) malloc(MAX_PAYLOAD_SIZE* sizeof(char));
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

    
    //std::cout << "Sent " << n << " bytes of packet " << packet.seqn << std::endl;
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

    }

    
    if (n == 0)
    {
        printf("Client disconnected.\n");
        return create_packet(-1,-1,-1,-1,NULL);
    }
    else if (n == -1)
    {
        perror("ERROR reading from socket");
        return create_packet(-1,-1,-1,-1,NULL);
    }

    return deserialize_packet(buffer);
}


int send_file(char* file_path, int socket){
    char packet_buffer[SIZE_PACKET];
    char* file_buffer;
    char* filename;
    long int numbytes = 0;
    int bytes_to_read;
    struct stat attrib;

    FILE * file;

    int n;

    // abrir arquivo
    file = fopen(file_path, "r+");
    if (file == NULL) {printf("File error.\n"); return -1;}

    // obter nome do arquivo
    filename = basename(file_path);

    // obter horario de modificação e criação do arquivo
    stat(file_path,&attrib);
    time_t modify_time = attrib.st_mtim.tv_sec;
    time_t create_time = attrib.st_ctim.tv_sec;


    // enviar packet de signal para recipiente
    Packet signal_packet = create_packet(PACKET_FILE_SIGNAL, 0, 1, strlen(filename), filename);
    send_packet(signal_packet,socket);
    free(signal_packet.payload);

    //Packet packet_mtime = create_packet(PACKET_FILE_MTIME, 1, 1, 1, );


    // obter comprimento do arquivo
    fseek(file, 0L, SEEK_END);
    numbytes = ftell(file);
    // resetar ponteiro do arquivo ao início do arquivo
    fseek(file, 0L, SEEK_SET);

    // não copiar mais bytes do que MAX_FILE_SIZE
    bytes_to_read = std::min(numbytes, (long)MAX_FILE_SIZE);

    // alocar memória para armazenar conteúdo do arquivo
    file_buffer = (char*)calloc(bytes_to_read, sizeof(char));
    if(file_buffer == NULL) {printf("Memory error.\n"); return -1;}

    // copia conteúdo do arquivo para a memória
    fread(file_buffer, sizeof(char), bytes_to_read, file);
    //fgets(file_buffer,MAX_FILE_SIZE, file); //só copia até encontrar um endline

    
    int packets_to_send = ceil((double)bytes_to_read / (double)MAX_PAYLOAD_SIZE);
    int seq = 0;
    int total_size = packets_to_send;

    //printf("bytes_to_read: %d\n", bytes_to_read);
    //printf("Need to send %d packets.\n", packets_to_send);

    // manda fragmentos do arquivo se for maior que MAX_PAYLOAD_SIZE
    while(bytes_to_read > MAX_PAYLOAD_SIZE){
        Packet packet = create_packet(PACKET_FILE_DATA,seq,total_size,MAX_PAYLOAD_SIZE,file_buffer+seq*MAX_PAYLOAD_SIZE);


        //printf("Trying to send file fragment number %d\n", seq);
        send_packet(packet,socket);

        seq++;
        bytes_to_read -= MAX_PAYLOAD_SIZE;
        // evitar um memory leak
        free(packet.payload);
    }
    // manda último fragmento se houver
    if(bytes_to_read > 0){
        Packet packet = create_packet(PACKET_FILE_DATA,seq,total_size,bytes_to_read,file_buffer+seq*MAX_PAYLOAD_SIZE);

        //printf("Payload of this packet: %s\n", packet.payload);
        //printf("strlen of payload: %d\n", strlen(packet.payload));
        //printf("bytes_to_read: %d\n", bytes_to_read);
        //printf("Trying to send file fragment number %d\n", seq);
        
        send_packet(packet,socket);
        //printf("Sent file fragment number %d\n", seq);
        //bzero(packet_buffer,SIZE_PACKET);

        // evitar um memory leak
        free(packet.payload);
    }

    fclose(file);



    // cria um pacote com o conteúdo lido do arquivo
    //Packet packet = create_packet(PACKET_FILE_DATA,-1,-1,-1,file_buffer); // TODO: colocar valores corretos

    //bzero(packet_buffer,SIZE_PACKET);

    // serializa o pacote
    //serialize_packet(packet, packet_buffer);

    // printf("Sending packet:\n");
    // print_packet(packet);

    // manda o pacote
	//int n = write(socket, packet_buffer,SIZE_PACKET);
    
    return 0;
}


int receive_file(char* buffer, int socket){
    int message;
    char packet_buffer[SIZE_PACKET];
    int bytes_read = 0;
    //int filesize = 0;

    //receber primeiro pacote


    Packet packet = receive_packet(socket);

    if(packet.type != PACKET_FILE_DATA){
        printf("Wrong packet type received in receive_file.\n");
        return -1;
    }

    int total_size = packet.total_size;
    //filesize += packet.length;
    //printf("Should receive %d packets.\n", total_size);
    //printf("Received fragment number %d\n", packet.seqn);


    memcpy(buffer+bytes_read, packet.payload, packet.length);
    bytes_read += packet.length;


    //recebe o resto dos pacotes
    for(int i=1 ; i < total_size; i++){
        Packet packet = receive_packet(socket);
        
        if(packet.type != PACKET_FILE_DATA){
            printf("Wrong packet type received in receive_file.\n");
            return -1;
        }
        //printf("Received fragment number %d\n", packet.seqn);
        //printf("Payload:\n%s\n",packet.payload);
        memcpy(buffer+bytes_read, packet.payload, packet.length);
        bytes_read += packet.length;
    }

    printf("Finished receiving file.");
    return bytes_read;
}