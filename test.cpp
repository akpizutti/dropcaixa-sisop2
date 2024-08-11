#include "users.hpp"
#include "packet.hpp"


#include <iostream> 
#include <vector>

int main(){
    User *test_user = new User("Paulo");
    //std::vector<User> connected_users;
    int a = MAX_PAYLOAD_SIZE;

    std::cout << test_user->get_username() << std::endl;
    char command;
    std::string path;

    std::cout << "enter command: ";
    std::cin >> command;
    std::cout << "enter filename: ";
    std::cin >> path;

    std::cout <<"\n path: " << path;



    return(0);
}