#ifndef _USERS_H_
#define _USERS_H_

#include <string>

class User {
    private:
        std::string username;
        int devices_connected;
    public:
        User(std::string username);
        int get_connected_devices();
        void set_connected_devices(int n);
        std::string get_username();
        
};



















#endif