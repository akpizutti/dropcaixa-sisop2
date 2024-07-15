#include "packet.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_packet(Packet packet){
    printf("Type: %d\n", packet.type);
    printf("Sequence: %d\n", packet.seqn);
    printf("Total size: %u\n", packet.total_size);
    printf("Length: %d\n", packet.length);
    printf("Payload: %s\n", packet.payload);
    return;
}

void print_packet_serialized(char* buffer){
    for (int i =0; i < PACKET_SIZE - MAX_PAYLOAD_SIZE; i++){
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
        strcpy(ret.payload, payload);
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

    strcpy(buffer+offset, data.payload);
    offset += MAX_PAYLOAD_SIZE;
    buffer[offset] = 0; //terminar payload com null (pode ser que precise terminar com outro caractere)

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
    strcpy(ret.payload, buffer+offset);

    //printf("hi\n");
    return ret;

}