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
    char* payload;          //Dados do pacote
} Packet;

// printa o conteúdo de uma struct de pacote, para debugging
void print_packet(Packet packet);
// printa o conteúdo de um pacote serializado, para debugging
void print_packet_serialized(char* buffer);

//inicializa uma struct de pacote (parece um construtor mas não é)
Packet create_packet(int type, int seqn, int total_size, int length, char* payload);

// transforma uma struct de pacote em uma sequência de bytes que pode ser enviada pelo socket (retorno é no parâmetro buffer)
// valores numéricos são representados em little-endian (byte menos significativo primeiro)
void serialize_packet(Packet data, char* buffer);

// transforma uma sequência de bits em uma struct de pacote
Packet deserialize_packet(char* buffer);




#endif