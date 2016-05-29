#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stack>
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

class Test : public Base {
    private:
        vector<string> vec;

    public:
        Test(vector<string> v) {
            vec = v;
        }
        bool evaluate() {
            if (vec[0] == "test") {
                vec.erase(vec.begin());
            }
            else {
                vec.erase(vec.begin());
                vec.erase(vec.end());
            }
            //first check if file exists
            struct stat buf;
            bool exists;
            string path = vec.at(vec.size() - 1);
            if (stat(path.c_str(), &buf) == 0) {
                exists = true;
            }
            else {
                exists = false;
            }
            if (!exists) {
                cout << "(False)" << endl; 
                return false;
            }
            if (exists) {
                vec.erase(vec.end());
                if (vec.size() == 0) {
                    cout << "(True)" << endl; 
                    return true;
                }
                else {
                    if (vec[0] == "-e") {
                        cout << "(True)" << endl;
                        return true;
                    }
                    else if (vec[0] == "-f") {
                        if (S_ISREG(buf.st_mode) != 0) {
                            cout << "(True)" << endl;
                            return true;
                        }
                        else {
                            cout << "(False)" << endl;
                            return false;
                        }
                    }
                    else if (vec[0] == "-d") {
                        if (S_ISDIR(buf.st_mode) != 0) {
                            cout << "(True)" << endl;
                            return true;
                        }
                        else {
                            cout << "(False)" << endl;
                            return false;
                        }
                    }
                }
            }
            cout << "(False)" << endl;
            return false;
        }
};

// Command Class that each command will inherit from - Ammar
class Command : public Base {
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
            if(commandVec[0] == "exit") {
                //Program stops if input is "exit" - Ammar
                exit(0);
            }
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
                else {
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

class Line : public Base {
    private:
        string line;

    public:
        Line(string l) {
            line = l;
        }
        bool evaluate() {
            bool returnThis;
            bool blank = false;
            if(line == ""){
                blank = true;
            }
            while(blank == false){
                string inputCommand = line.substr(0, line.find('#', 1));        
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
                //print connectors
                //cout << "elements of \"connectors\" vector: "; printVec(connectors);
                //parses inputs of multiple commands
                vector<string> myCommands = parser(inputCommand, "||&&;");
                //print myCommands
                //cout << "elements of \"myCommands\" vector: "; printVec(myCommands);
                Base* firstCommand;
                Base* firstTest;
                vector<string> command1 = parser(myCommands.at(0), " ");
                //print command1
                //cout << "elements of \"command1\" vector: "; printVec(command1);
                if ((command1[0] == "test") || (command1[0] == "[")) {
                    //run test
                    firstTest = new Test(command1);
                    returnThis = firstTest->evaluate();
                }
                else {
                    //run command
                    firstCommand = new Command(command1);
                    returnThis = firstCommand->evaluate();
                }
                //here we have to make desicions base on the temp2 = true or false    
                for(unsigned int i = 0; i < connectors.size(); i ++){
                    Base* NxtCommand;
                    Base* NxtTest;
                    bool runningTest;
                    //this will make a comand ready for the execvp funct
                    vector<string> CommandReady = parser(myCommands.at(i + 1), " ");
                    //print CommandReady
                    //cout << "elements of \"CommandReady\" vector: "; printVec(CommandReady);
                    if (connectors.at(i) == "&&") { //for and
                        if ((CommandReady[0] == "test") || (CommandReady[0] == "[")) {
                            NxtTest = new And(returnThis, new Test(CommandReady));
                            runningTest = true;
                        }
                        else {
                            NxtCommand = new And(returnThis, new Command(CommandReady));
                            runningTest = false;
                        }
                    }
                    else if (connectors.at(i) == "||") { //for or
                        if ((CommandReady[0] == "test") || (CommandReady[0] == "[")) {
                            NxtTest = new Or(returnThis, new Test(CommandReady));
                            runningTest = true;
                        }
                        else {
                            NxtCommand = new Or(returnThis, new Command(CommandReady));
                            runningTest = false;
                        }
                    }
                    else if (connectors.at(i) == ";") {//for semicolon
                        if ((CommandReady[0] == "test") || (CommandReady[0] == "[")) {
                            NxtTest = new Semicolon(returnThis, new Test(CommandReady));
                            runningTest = true;
                        }
                        else {
                            NxtCommand = new Semicolon(returnThis, new Command(CommandReady));
                            runningTest = false;
                        }
                    }
                    if (runningTest) {
                        returnThis = NxtTest->evaluate();
                    }
                    else {
                        returnThis = NxtCommand->evaluate();
                    }
                }
                blank = true; //this means done with this command and wants next one
            }
            return returnThis;
        }
};

vector<string> split(string str, char delimiter) {
    vector<string> internal;
    stringstream ss(str);
    string tok;
    while (getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }
    return internal;
}

void printVec(vector<string> &v) {
    for (unsigned int i = 0; i < v.size(); ++i) {
        if (i == (v.size() - 1)) {
            cout << v[i];
        }
        else {
            cout << v[i] << ", ";
        }
    }
    cout << endl;
}

bool isAConnector(string str) {
    if ((str != "&&") && (str != "||") && (str != ";")) {
        return false;
    }
    else {
        return true;
    }
}

void infix2postfix(vector<string> &infix, vector<string> &postfix) {
    stack<string> s;
    //int weight;
    unsigned int i = 0;
    string str;
    string command;
    while (i < infix.size()) {
        str = infix.at(i);
        if (str == "(") {
            s.push("(");
            i++;
            continue;
        }
        if (str == ")") {
            while (!s.empty() && s.top() != "(") {
                postfix.push_back(s.top());
                s.pop();
            }
            if (!s.empty()) {
                s.pop();
            }
            i++;
            continue;
        }
        if (!isAConnector(str)) {
            while (!isAConnector(str)) {
                command += str;
                command += ' ';
                i++;
            }
            postfix.push_back(command);
            command = "";
            i++;
        }
        else {
            if (s.empty()) {
                s.push(str);
            }
            else {
                while (!s.empty() && s.top() != "(") {
                    postfix.push_back(s.top());
                    s.pop();
                }
                s.push(str);
            }
        }
        i++;
    }
    while (!s.empty()) {
        postfix.push_back(s.top());
        s.pop();
    }
}

int main () {
    
    string commandInput = "";
    
    Base* theLine;
    while (true) {
        string login = getlogin();
        char hostname[100];
        gethostname(hostname, 100);
        cout << "[" << login << "@" << hostname << "] $ ";
        getline(cin, commandInput);
        trim(commandInput);
        theLine = new Line(commandInput);
        if ((commandInput.find("(") != string::npos) && (commandInput.find(")") != string::npos)) {
            //there is precedence
            cout << "This input uses precedence" << endl;
            vector<string> tokens = split(commandInput, ' ');
            printVec(tokens);
            vector<string> parsedVector;
            string temp;
            vector<string> postfix;
            for (unsigned int i = 0; i < tokens.size(); ++i) {
                temp = tokens.at(i);
                if (tokens.at(i).at(0) == '(') {
                    //cout << tokens.at(i) << endl;
                    temp = temp.substr(1, temp.size() - 1);
                    //cout << temp << endl;
                    parsedVector.push_back("(");
                    parsedVector.push_back(temp);
                    //cout << "Iteration " << i + 1 << " "; printVec(parsedVector); 
                }
                else if (tokens.at(i).at(tokens.at(i).size() - 1) == ';') {
                    if (temp.at(temp.size() - 2) == ')')  {
                        temp = temp.substr(0, temp.size() - 2);
                        parsedVector.push_back(temp);
                        parsedVector.push_back(")");
                        parsedVector.push_back(";");
                    }
                    else {
                        temp = temp.substr(0, temp.size() - 1);
                        parsedVector.push_back(temp);
                        parsedVector.push_back(";");
                    }
                }
                else if (temp.at(temp.size() - 1) == ')') {
                    temp = temp.substr(0, temp.size() - 1);
                    parsedVector.push_back(temp);
                    parsedVector.push_back(")");
                }
                else {
                    parsedVector.push_back(tokens.at(i));
                    //cout << "Iteration " << i + 1 << " "; printVec(parsedVector); 
                }
            }
            printVec(parsedVector);
            infix2postfix(parsedVector, postfix);
            printVec(postfix);
        }
        else {
            theLine->evaluate();
        }
    }
    return 0;
}
