#include "users.hpp"
#include <iostream> 

int main(){
    User *test_user = new User("Paulo");

    std::cout << test_user->get_username() << std::endl;
}