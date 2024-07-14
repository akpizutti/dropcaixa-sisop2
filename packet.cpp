#include "packet.hpp"
#include <stdio.h>
#include <string.h>

void serialize_packet(Packet data, char* buffer){
    int offset = 0;
    strcpy(buffer+offset, reinterpret_cast<char*>(&data.type));
    offset += sizeof(data.type);
    strcpy(buffer+offset, reinterpret_cast<char*>(&data.seqn));
    offset += sizeof(data.seqn);
    strcpy(buffer+offset, reinterpret_cast<char*>(&data.total_size));
    offset += sizeof(data.total_size);
    strcpy(buffer+offset, reinterpret_cast<char*>(&data.length));
    offset += sizeof(data.length);

    for (int i =0; i < offset; i++){
        printf("%hhu, %d\n",buffer[i], sizeof(buffer[i]));
    }



    strcpy(buffer+offset, data.payload);
    offset += MAX_PAYLOAD_SIZE;
    buffer[offset] = 0;

    printf("offset: %d\n", offset);
    printf("buffer strlen in packet.cpp: %d",strlen(buffer));
    return;
}