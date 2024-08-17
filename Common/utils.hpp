#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

int save_file(std::string path, int filesize, char* buffer);

void long_to_bytes(long number, char* buffer);
char* long_to_bytes(long number);
long bytes_to_long(char* bytes);
void print_16bytes(char* buffer);
void create_sync_dir(std::string username);
std::string get_sync_dir_relative_path(std::string username);
int bytes_to_int(char* bytes);
char* int_to_bytes(int number);


#endif