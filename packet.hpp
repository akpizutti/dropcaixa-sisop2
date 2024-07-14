#ifndef _PACKET_H_
#define _PACKET_H_

#include <cstdint>

#define MAX_PAYLOAD_SIZE 256
#define PACKET_SIZE 3 * sizeof(uint16_t) + sizeof(uint32_t) + MAX_PAYLOAD_SIZE * sizeof(char)

typedef struct packet{
    uint16_t type;          //Tipo do pacote
    uint16_t seqn;          //Número de sequência
    uint32_t total_size;    //Número total de fragmentos
    uint16_t length;        //Comprimento do payload
    char* payload;    //Dados do pacote
} Packet;

void serialize_packet(Packet data, char* buffer);
Packet create_packet(int type, int seqn, int total_size, int length, char* payload);


#endif