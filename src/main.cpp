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
class Base  {
    public:
        Base(){};
        //Function Inherited by each class
        virtual bool evaluate() = 0;
};

// Command Class that each command will inherit from
class Command: public Base {
    private:
    
    //Vector of commands
    vector<string> commandVec;

    public:
    
    //Contructor to take in vector and set it to commands vectors
    Command(vector<string>s){
        commandVec = s;
    }
    bool evaluate(){
        
        //exit if cmd is "exit"
        if(commandVec[0] == "exit")
            exit(0);
        
        //this chunk is to format the vector in the way we want it
        vector<char *> temp2;
        for(unsigned int i = 0; i < commandVec.size(); i++) {
            temp2.push_back(const_cast<char *>(commandVec.at(i).c_str()));
        }
        temp2.push_back('\0'); //'\0 is to make sure there is a null char in c-str'
        char** arr = &temp2[0];


        //here we will use fork() so we can do multiple process at once
        int status;
        pid_t pid = fork();
        if (pid < 0) { //to chck if fork failed
            perror("FORK has FAILED");
            exit(1);
        }
        else if (pid == 0) {
            //if it reaches here, you can pass into execvp
            //execvp will do all the work for you
            execvp(const_cast<char *>(arr[0]), arr);
            //if it reaches here there is some error
            exit(127); // exit 127 "command not found"
        }
        else if(pid > 0){
            //have to wait until child finishes
            // use wait pid or wait ???? waitpid(pid, &status, 0);
            wait(&status);
            if(wait(&status) != -1){
                perror("ERROR: wait");
            }
            if(WIFEXITED(status)){
                if(WEXITSTATUS(status) == 0) {
                    
                    //program is succesful
                    return true;
                }
                else {
                    
                    //this return is false, then the program failed but exiting was normal
                    return false;
                }
            }
            else{
                
                //the program messed up and exited abnormally
                perror("EXIT: ABNORMAL CHILD");
                return false;
            }
        }
        return false;            
    } 
};

class Connectors : public Base {
    private:
        Connectors(){};
    
    protected:
        bool leftCommand;
        Base* rightCommand;
};

vector<string> parser(string toSplit, const char* delimiters) {
    char* toTokenize = new char[toSplit.size() + 1];
    strcpy(toTokenize, toSplit.c_str());
    toTokenize[toSplit.size() + 1] = '\0';
    char* cutThis;
    //begin parsing
    cutThis = strtok(toTokenize, delimiters);
    vector<string> returnThis;
    while (cutThis != NULL) {
        string currWord(cutThis);
        trim(currWord);
        returnThis.push_back(currWord);
        cutThis = strtok(NULL, delimiters);
    }
    return returnThis;
}

int main () {
    
    //take user input
    string initialCommand = "";
    
    while (true) { //infinite loop is there until exit is found
        //this is the extra credit part
        string login = getlogin();
        char hostname[100];
        gethostname(hostname, 100);
        cout << "[" << login << "@" << hostname << "] $ ";
        
        getline(cin, initialCommand);
        trim(initialCommand);
        bool noCMD = false;
        if(initialCommand == ""){
            noCMD = true;
        }
        while(noCMD == false){
            //FIXME:: NEED TO ADD STUFF HERE
            
            noCMD = true; //this means done with this command and wants next one
        }
        
        
    }
    
    return 0;
}
