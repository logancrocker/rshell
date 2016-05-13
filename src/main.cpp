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

//Virtual Base Class
class Base{
public:
    Base(){};
    //Function Inherited by each class
    virtual bool evaluate() = 0;
};

// Command Class that each command will inherit from
class Command: public Base{
private:
    //Vector of commands
    vector<string> commandVec;
public:
    //Contructor to take in vector and set it to commands vectors
    Command(vector<string>s){
        commandVec = s;
    }
    
};

class Connectors:public Base{
private:
    Connectors(){};
    
protected:
    bool leftCommand;
    Base* rightCommand;
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
