#include "users.hpp"


User::User(std::string username){
    this->username = username;
    this->devices_connected = 0;
}

std::string User::get_username(){
    return this->username;
}

int User::get_connected_devices(){
    return this->devices_connected;
}

void User::set_connected_devices(int n){
    this->devices_connected = n;
}