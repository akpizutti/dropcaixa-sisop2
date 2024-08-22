#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

struct connection_info
{
	int id;
	int socket;
	std::string username;
};

int save_file(std::string path, int filesize, char* buffer);

// these functions are made for little-endian systems
char* long_to_bytes(long number);
long bytes_to_long(char* bytes);
char* int_to_bytes(int number);
int bytes_to_int(char* bytes);




void print_16bytes(char* buffer);
void create_sync_dir(std::string username);
std::string get_sync_dir_relative_path(std::string username);



#endif