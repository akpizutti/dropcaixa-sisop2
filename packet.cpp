#include "packet.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    strcpy(buffer+offset, data.payload);
    offset += MAX_PAYLOAD_SIZE;
    buffer[offset] = 0;
    return;
}


Packet create_packet(int type, int seqn, int total_size, int length, char* payload){
    Packet ret;
    ret.type = type;
    ret.seqn = seqn;
    ret.total_size = total_size;
    ret.length = length;
    ret.payload = (char*) malloc(MAX_PAYLOAD_SIZE* sizeof(char));
    strcpy(ret.payload, payload);
    return ret;
}