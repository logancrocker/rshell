#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/socket.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;

class Base{
    public:
        Base(){};
        virtual bool evaluate() = 0;
};

class Command: public Base{
    private:
        vector<string> commandVec;
    public:
        Command(vector<string>s){
            commandVec = s;
        }
  
};

int main () {
    
    //take user input
    string initialCommand = "";
    while (true) {
        string login = getlogin();
        char hostname[100];
        gethostname(hostname, 100);
        cout << "[" << login << "@" << hostname << "] $ ";
        getline(cin, initialCommand);
        trim(initialCommand);

    }

    return 0;
}
