#ifndef _PACKET_H_
#define _PACKET_H_

#include <cstdint>
#include <iostream>
#include <string>

// definição de tipos de pacotes
#define PACKET_FILE_SIGNAL 1 // pacote que indica que o próximo será um PACKET_FILE_NAME. payload contém nome do arquivo
#define PACKET_FILE_DATA 2   // pacote que contém um arquivo ou fragmento de um arquivo



#define MAX_PAYLOAD_SIZE 256
#define SIZE_PACKET 3 * sizeof(uint16_t) + sizeof(uint32_t) + MAX_PAYLOAD_SIZE * sizeof(char)
#define MAX_FILE_SIZE 65536

typedef struct packet{
    uint16_t type;          //Tipo do pacote
    uint16_t seqn;          //Número de sequência
    uint32_t total_size;    //Número total de fragmentos
    uint16_t length;        //Comprimento do payload
    char* payload;          //Dados do pacote
} Packet;

class Packet{
    private:
        uint16_t type;          //Tipo do pacote
        uint16_t seqn;          //Número de sequência
        uint32_t total_size;    //Número total de fragmentos
        uint16_t length;        //Comprimento do payload
        std::array<char, MAX_PAYLOAD_SIZE> payload;          //Dados do pacote
    public:
        //construtores
        Packet(uint16_t type, uint16_t seqn, uint32_t total_size, uint16_t length, char* payload);

        // transforma uma instancia de pacote em uma sequência de bytes que pode ser enviada pelo socket
        void serialize_packet(char* buffer);
        // transforma uma sequência de bytes em uma instancia de pacote
        Packet deserialize_packet(char* serialized);

        friend std::ostream& operator<<(std::ostream&, const Packet& p);

        
};



// printa o conteúdo de um pacote serializado, para debugging
void print_packet_serialized(char* buffer);



// envia um arquivo para o socket especificado
int send_file(char* filePath, int socket);

// recebe um arquivo e coloca seu conteúdo em buffer
int receive_file(char* buffer, int socket);





#endif