#include "utils.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

int save_file(std::string path, int filesize, char *buffer)
{
    FILE *file = fopen(path.c_str(), "wb");
    if (file == NULL)
    {
        std::cout << "Erro ao abrir arquivo" << std::endl;
        std::cout << "path: " << path << std::endl;
        return -1;
    }

    //printf("File received: \n%s\n", buffer);
    fwrite(buffer, 1, filesize, file);


    fclose(file);
    return 0;
}

void long_to_bytes(long number, char* buffer){
    buffer = (char*)calloc(sizeof(long), sizeof(char));
    buffer[0] = (char)(number & 0xff);
    buffer[1] = (char)((number >> 8) & 0xff);
    buffer[2] = (char)((number >> 16) & 0xff);
    buffer[3] = (char)((number >> 24) & 0xff);
    // std::cout << "\nin long_to_bytes(): \n"; //<< (unsigned short)buffer[0] << " " << (unsigned short)buffer[2] << " "<< (unsigned short)buffer[3] << " "<< (unsigned short)buffer[4] << std::endl;
    // printf("%hhu\n",buffer[0]);
    // printf("%hhu\n",buffer[1]);
    // printf("%hhu\n",buffer[2]);
    // printf("%hhu\n",buffer[3]);
    // std::cout << "with input  " << number << std::endl;
    return;
}

char* long_to_bytes(long number){
    char *buffer = (char*)calloc(sizeof(long), sizeof(char));
    buffer[0] = (char)(number & 0xff);
    buffer[1] = (char)((number >> 8) & 0xff);
    buffer[2] = (char)((number >> 16) & 0xff);
    buffer[3] = (char)((number >> 24) & 0xff);
    // std::cout << "\nin long_to_bytes() overload: \n"; //<< (unsigned short)buffer[0] << " " << (unsigned short)buffer[2] << " "<< (unsigned short)buffer[3] << " "<< (unsigned short)buffer[4] << std::endl;
    // printf("%hhu\n",buffer[0]);
    // printf("%hhu\n",buffer[1]);
    // printf("%hhu\n",buffer[2]);
    // printf("%hhu\n",buffer[3]);
    // std::cout << "with input  " << number << std::endl;
    return buffer;
}


long bytes_to_long(char* bytes){
    // std::cout << "\nin bytes_to_long(): \n"; //<< (unsigned short)buffer[0] << " " << (unsigned short)buffer[2] << " "<< (unsigned short)buffer[3] << " "<< (unsigned short)buffer[4] << std::endl;
    // printf("%hhu\n",bytes[0]);
    // printf("%hhu\n",bytes[1]);
    // printf("%hhu\n",bytes[2]);
    // printf("%hhu\n",bytes[3]);
    long ret = (bytes[0] & 0xff) + ((bytes[1] << 8) & 0xff00) + ((bytes[2] << 16) & 0xff0000) + ((bytes[3] << 24) & 0xff000000);
    // std::cout << "\nret = " << ret << std::endl;
    return ret;
}

void print_16bytes(char* buffer){

    for(int i=0; i<16; i++){
        printf("%hhu\n",buffer[i]);
    }
    // printf("%hhu\n",buffer[0]);
    // printf("%hhu\n",buffer[1]);
    // printf("%hhu\n",buffer[2]);
    // printf("%hhu\n",buffer[3]);
    return;
}


void create_sync_dir(std::string username){
    std::string sync_dir = "./sync_dir_";
    std::string sync_dir_user;
    sync_dir_user = sync_dir + username;
    struct stat sb;

    if (stat(sync_dir_user.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
				{
					//printf("Directory exists\n");
				} else{
					//printf("Creating sync_dir...\n");
					std::cout << "Creating sync_dir for " << username << std::endl;
					mkdir(sync_dir_user.c_str(), 0700);
					
					printf("sync_dir created!\n");
				}
}