#include "utils.hpp"
#include <iostream>

int save_file(std::string path, int filesize, char *buffer)
{
    FILE *file = fopen(path.c_str(), "wb");
    if (file == NULL)
    {
        std::cout << "nao abriu o arquivo" << std::endl;
        std::cout << "path: " << path << std::endl;
        return -1;
    }

    std::cout << "\nhello\n";

    //printf("File received: \n%s\n", buffer);
    fwrite(buffer, 1, filesize, file);


    fclose(file);
    return 0;
}