#include "users.hpp"
#include "packet.hpp"
#include "utils.hpp"


#include <iostream> 
#include <vector>
#include <sys/stat.h>

int main(){
    User *test_user = new User("Paulo");
    //std::vector<User> connected_users;
    int a = MAX_PAYLOAD_SIZE;



    std::cout << test_user->get_username() << std::endl;



    // std::string path;
    // struct stat attrib;
    // std::cout << "enter filename: ";
    // std::cin >> path;

    // stat(path.c_str(), &attrib);

    // std::cout << sizeof(attrib);

    char* buffer;
    //long_to_bytes((long)36774, buffer);
    buffer = long_to_bytes((long)36774);
    printf("%hhu\n",buffer[0]);
    printf("%hhu\n",buffer[1]);
    printf("%hhu\n",buffer[2]);
    printf("%hhu\n",buffer[3]);
    std::cout << "converting back: " << bytes_to_long(buffer) << std::endl ;
    printf("%hhu\n",buffer[0]);
    printf("%hhu\n",buffer[1]);
    printf("%hhu\n",buffer[2]);
    printf("%hhu\n",buffer[3]);


    return(0);
}