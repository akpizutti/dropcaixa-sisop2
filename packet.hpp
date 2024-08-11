#ifndef _PACKET_H_
#define _PACKET_H_

#include <cstdint>

// definição de tipos de pacotes
#define PACKET_FILE_SIGNAL 1 // pacote que indica que o próximo será um PACKET_FILE_NAME. payload contém nome do arquivo
#define PACKET_FILE_DATA 2   // pacote que contém um arquivo ou fragmento de um arquivo
#define PACKET_USER_ID 3

#define MAX_PAYLOAD_SIZE 256
#define SIZE_PACKET 3 * sizeof(uint16_t) + sizeof(uint32_t) + MAX_PAYLOAD_SIZE * sizeof(char)
#define MAX_FILE_SIZE 65536

typedef struct packet
{
    uint16_t type;       // Tipo do pacote
    uint16_t seqn;       // Número de sequência
    uint32_t total_size; // Número total de fragmentos
    uint16_t length;     // Comprimento do payload
    char *payload;       // Dados do pacote
} Packet;

// printa o conteúdo de uma struct de pacote, para debugging
void print_packet(Packet packet);
// printa o conteúdo de um pacote serializado, para debugging
void print_packet_serialized(char *buffer);

// inicializa uma struct de pacote (parece um construtor mas não é)
Packet create_packet(int type, int seqn, int total_size, int length, char *payload);

// transforma uma struct de pacote em uma sequência de bytes que pode ser enviada pelo socket (retorno é no parâmetro buffer)
// valores numéricos são representados em little-endian (byte menos significativo primeiro)
void serialize_packet(Packet data, char *buffer);

// transforma uma sequência de bits em uma struct de pacote
Packet deserialize_packet(char *buffer);

int send_packet(Packet packet, int socket);

Packet receive_packet(int socket);

// envia um arquivo para o socket especificado
int send_file(char *filePath, int socket);

// recebe um arquivo e coloca seu conteúdo em buffer
int receive_file(char *buffer, int socket);

#endif