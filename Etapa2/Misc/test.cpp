#include "../Common/users.hpp"
#include "../Common/packet.hpp"
#include "../Common/utils.hpp"


#include <iostream> 
#include <vector>
#include <sys/stat.h>
#include <math.h>



using namespace std;

vector<User *> users;

int main(){
    User *paulo = new User("Paulo");
    User *carlos = new User("Carlos");
    User *gabriel = new User("Gabriel");

    //std::vector<User> connected_users;
    //int a = MAX_PAYLOAD_SIZE;



    users.push_back(paulo);
    //users.push_back(carlos);
    //users.push_back(gabriel);

    for (int i = 0; i < users.size(); i++)
	{
		std::cout << users[i]->get_username() << std::endl;
	}

    cout << "deletando\n";

    remove_user("Paulo", &users);

    for (int i = 0; i < users.size(); i++)
	{
		std::cout << users[i]->get_username() << std::endl;
	}


    //std::cout << test_user->get_username() << std::endl;



    // std::string path;
    // struct stat attrib;
    // std::cout << "enter filename: ";
    // std::cin >> path;

    // stat(path.c_str(), &attrib);

    // std::cout << sizeof(attrib);

    // char* buffer;
    // //long_to_bytes((long)36774, buffer);
    // buffer = long_to_bytes((long)36774);
    // printf("%hhu\n",buffer[0]);
    // printf("%hhu\n",buffer[1]);
    // printf("%hhu\n",buffer[2]);
    // printf("%hhu\n",buffer[3]);
    // std::cout << "converting back: " << bytes_to_long(buffer) << std::endl ;
    // printf("%hhu\n",buffer[0]);
    // printf("%hhu\n",buffer[1]);
    // printf("%hhu\n",buffer[2]);
    // printf("%hhu\n",buffer[3]);


    // FILE* file = fopen("a.txt", "r+");
    // if (file == NULL) {printf("File error.\n"); return -1;}
    // //cout << "fopen returned " << file << endl;
    // //printf("fopen returned %ld", file);
    // cout << "sizeof FILE*: " << sizeof(file) << endl;
    // cout << "sizeof long: " << sizeof(long) << endl;
    // cout << "sizeof int : " << sizeof(int) << endl;


    // unsigned long a = 0xff00ff00ff00ff00;
    // char* a_bytes = long_to_bytes(a);
    // for(int i=0;i<sizeof(long);i++){
    //     printf("%hhu\n",a_bytes[i]);
    // }

    // cout << "result of conversion: " << (unsigned long)bytes_to_long(a_bytes);





    return(0);
}