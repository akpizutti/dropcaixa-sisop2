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



void remove_user(std::string username, std::vector<User *> *users){
    // int length = users.size();
    // for(int i=0; i<length; i++){
    //     if(users[i]->get_username() == username){
    //         users.erase(i);
    //     }
    // }


    if(users->size() == 0){
        return;
    } else if(users->size() == 1){
        auto iter = users->begin();
        if((*iter)->get_username() == username){
            users->erase(iter);
        }
    } else{
        for(auto iter = users->begin(); iter != users->end(); iter++){
            if((*iter)->get_username() == username){
                users->erase(iter);
            }
    }
    }
    return;

}