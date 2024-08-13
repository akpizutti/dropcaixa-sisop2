#include "users.hpp"
#include "packet.hpp"


#include <iostream> 
#include <vector>
#include <sys/stat.h>

int main(){
    User *test_user = new User("Paulo");
    //std::vector<User> connected_users;
    int a = MAX_PAYLOAD_SIZE;



    std::cout << test_user->get_username() << std::endl;



    std::string path;
    struct stat attrib;
    std::cout << "enter filename: ";
    std::cin >> path;

    stat(path.c_str(), &attrib);

    std::cout << sizeof(attrib);



    return(0);
}