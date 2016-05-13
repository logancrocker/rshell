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

//Virtual Base Class - Ammar
class Base  {
public:
    Base(){};
    //Function Inherited by each class - Ammar
    virtual bool evaluate() = 0;
};

// Command Class that each command will inherit from - Ammar
class Command: public Base {
private:
    
    //Vector of commands - Ammar
    vector<string> commandVec;
    
public:
    
    //Contructor to take in vector and set it to commands vectors
    Command(vector<string>s){
        commandVec = s;
    }
    bool evaluate(){
        
        //exit if cmd is "exit"
        if(commandVec[0] == "exit")
            //Program stops if input is "exit" - Ammar
            exit(0);
        
        //this chunk is to format the vector in the way we want it
        vector<char *> temp2;
        for(unsigned int i = 0; i < commandVec.size(); i++) {
            temp2.push_back(const_cast<char *>(commandVec[i].c_str()));
        }
        temp2.push_back('\0'); //'\0 is to make sure there is a null char in c-str'
        char** arrChar = &temp2[0];
        
        
        //here we will use fork() so we can do multiple process at once
        int status;
        pid_t pid = fork();
        if (pid < 0) { //to chck if fork failed
            perror("FAILED");
            exit(1);
        }
        else if (pid == 0) {
            //if it reaches here, you can pass into execvp
            //execvp will do all the work for you
            execvp(const_cast<char *>(arrChar[0]), arrChar);
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
    public:
        Connectors(){};
    
    protected:
        bool leftCommand; //command b4 the connector
        Base* rightCommand; //command @ft3r the connect0r
};

//will always attempt to run rightCommand
class Semicolon : public Connectors {
    public:
        Semicolon(bool l, Base* r){
            leftCommand = l; 
            rightCommand = r;
        }
        bool evaluate() {
            return rightCommand->evaluate();
        }
};

//will run the rightcommand if leftcommand succededs
class And : public Connectors{
public:
    And(bool l, Base* r){
        leftCommand = l; rightCommand = r;
    }
    bool evaluate(){
        if(leftCommand)
            return rightCommand->evaluate();
        return false;
    }
};

//will run the rightCommand if the LeftCommand fails
class Or : public Connectors{
public:
    Or(bool l, Base* r){
        leftCommand = l; rightCommand = r;
    }
    
    //Return if it evaluated or not
    bool evaluate(){
        if(!leftCommand)
            return rightCommand->evaluate();
        return false;
    }
};

//This Function takes the user input and parses it returns us a vector of strings - Ammar
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
    string commandInput = "";
    
    while (true) { //Keep checking until exit is found
        //this is the extra credit part
        string login = getlogin();
        char hostname[100];
        gethostname(hostname, 100);
        
        //display login and host name and waits for user input
        
        cout << "[" << login << "@" << hostname << "] $ ";
        
        getline(cin, commandInput);
        // Gets rid of leading and ending uneeded space - Ammar
        trim(commandInput);
        bool blank = false;
        if(commandInput == ""){
            blank = true;
        }
        while(blank == false){
            string inputCommand = commandInput.substr(0, commandInput.find('#', 1));        
            vector<string> connectors;
            //filling vector of connectors
            for(unsigned int i = 0; i < inputCommand.length(); i++){
                if(inputCommand[i] == '|'){
                    if(inputCommand[i + 1] == '|'){
                        connectors.push_back("||");        
                    } 
                }
                else if(inputCommand[i] == '&'){
                    if(inputCommand[i + 1] == '&'){
                        connectors.push_back("&&");
                    }
                }
                else if(inputCommand[i] == ';'){
                    connectors.push_back(";");
                }
            }
            
            //parses inputs of multiple commands
            vector<string> myCommands = parser(inputCommand, "||&&;");
            
            vector<string> command1 = parser(myCommands.at(0), " ");
            Base* firstCommand = new Command(command1);
            bool temp2 = firstCommand->evaluate(); //run first command and see if fail or succesful
            
            //here we have to make desicions base on the temp2 = true or false    
            for(unsigned int i = 0; i < connectors.size(); i ++){
                
                Base* NxtCommand;
                //this will make a comand ready for the execvp funct
                vector<string> CommandReady = parser(myCommands.at(i + 1), " ");
                if (connectors.at(i) == "&&") { //for and
                    NxtCommand = new And(temp2, new Command(CommandReady));
                }
                else if (connectors.at(i) == "||") { //for or 
                    NxtCommand = new Or(temp2, new Command(CommandReady));
                }
                else if (connectors.at(i) == ";") {//for semicolon
                    NxtCommand = new Semicolon(temp2, new Command(CommandReady));                    
                }
                NxtCommand->evaluate();
            }
            blank = true; //this means done with this command and wants next one
        }
    }
    return 0;
}
