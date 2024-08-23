#ifndef _PACKET_H_
#define _PACKET_H_

#include <cstdint>
#include <string>
#include <sys/stat.h>

// definição de tipos de pacotes
#define PACKET_FILE_SIGNAL 1 // pacote que indica que o próximo será um PACKET_FILE_NAME. payload contém nome do arquivo
#define PACKET_FILE_DATA 2   // pacote que contém um arquivo ou fragmento de um arquivo
#define PACKET_FILE_MTIME 3  // timestamp de modificação do arquivo
#define PACKET_FILE_CTIME 4  // timestamp de criação do arquivo
#define PACKET_FILE_LENGTH 5 // comprimento em bytes do arquivo a ser enviado
#define PACKET_USER_ID 6
#define PACKET_GET_SYNC_DIR 7
#define PACKET_FILE_COUNT 8
#define PACKET_DELETE_FILE 9
#define PACKET_SIGNAL_SYNC 10


#define PACKET_REJECT 254
#define PACKET_ERROR 255

#define MAX_PAYLOAD_SIZE 256
#define SIZE_PACKET 3 * sizeof(uint16_t) + sizeof(uint32_t) + MAX_PAYLOAD_SIZE * sizeof(char)
#define MAX_FILE_SIZE 1048576

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
int send_file(std::string filePath, int socket);

// recebe um arquivo e coloca seu conteúdo em buffer
int receive_file(char *buffer, int socket);

// cliente pede um arquivo e o servidor manda
int download_file(char* buffer, int socket, std::string filename);

void send_error(int socket);

#endif