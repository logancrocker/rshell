#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;
//Working ??

//Virtual Base Class - Ammar
class Base  {
public:
    Base(){};
    //Function Inherited by each class - Ammar
    virtual bool evaluate() = 0;
};


class Test{
private:
    int flagNum;
public:
    
    bool found;
    
    
    Test(vector<string> argsTest){
        if (argsTest[0] =="-e") {
            argsTest.erase(argsTest.begin());
            flagNum =1;
        }
        else if (argsTest[0] =="-f") {
            argsTest.erase(argsTest.begin());
            flagNum =2;
        }
        else if (argsTest[0] =="-d") {
            argsTest.erase(argsTest.begin());
            flagNum =3;
        }
        else{
            flagNum = 1;
        }
        
        vector<char *> charVec;
        charVec.push_back(const_cast<char *>(argsTest[0].c_str()));
        charVec.push_back(('\0'));
        char** charVec_two = &charVec[0];
                          struct stat statStruct;
                          
                          if(stat(const_cast<char *>(charVec_two[0]), &statStruct)<0){
                              found = false;
                          }
                          else{
                              if (flagNum == 1) {
                                  found = true;
                              }
                              
                              else if(flagNum == 2) {
                                  (S_ISREG(statStruct.st_mode)) ? found = true : found = false;
                              }
                              else if (flagNum == 3) {
                                  (S_ISDIR(statStruct.st_mode)) ? found = true : found = false;
                              }
                              else {
                                  cout << "Error" << endl;
                              }
                        
                          }
        
        
    }
    
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
                          
                          unsigned perEnds(string commandInput, int a){
                              stack<char> charStack;
                              unsigned i =a;
                              charStack.push('f');
                              
                              i++;
                              for(; i < commandInput.size(); i++)
                              {
                                  if(commandInput.at(i)== '('){
                                      charStack.push('(');
                                  }
                                  else if(commandInput.at(i) == ')'){
                                      char open = charStack.top();
                                      charStack.pop();
                                      if(charStack.empty() && open =='f')
                                      {
                                          return i;
                                      }
                                  }
                              }
                              return i;
                          }
                          
                          string parsePer(string commandInput){
                              stack<char> charStack;
                              bool isBool = 0;
                              do{
                                  trim(commandInput);
                                  
                                  if(commandInput.find('(') !=0){
                                      return commandInput;
                                  }
                                  else if(perEnds(commandInput, 0) == commandInput.size()-1){
                                      commandInput.erase(0,1);
                                      commandInput.erase(commandInput.size()-1);
                                      if(perEnds(commandInput, 0)== commandInput.size()-1){
                                          isBool = 1;
                                      }
                                      else{
                                          return commandInput;
                                      }
                                
                                  }
                                  
                              
                              else{
                                  return commandInput;
                              }
                              }
                              while(isBool == 1);
                              return commandInput;
                          }
                          
                          
                          void perCheck(string commandInput){
                              stack<char> charStack;
                              
                              for(unsigned int i =0; i < commandInput.size();i++)
                              {
                                  if(commandInput.at(i)== '('){
                                      charStack.push('(');
                                  }
                                  else if(commandInput.at(i)== ')'){
                                      if(!charStack.empty()){
                                          charStack.pop();
                                      }
                                      else{
                                          cout << "Error";
                                          
                                          exit(0);
                                      }
                                  }
                              }
                              if(!charStack.empty()){
                                  cout << "Error";
                                  exit(0);
                              }
                          }
                          
                          
                          class Chunks:public Base{
                          private:
                              string commandInput;
                              vector<bool> track;
                              bool isNested;
                          public:
                              Chunks(string s){
                                  commandInput =s;
                              }
                              
                              bool evaluate()
                              {
                                  trim(commandInput);
                                  commandInput = parsePer(commandInput);
                                  if(commandInput == ""){
                                      return 1;
                                  }
                                  isNested = 0;
                                  for(unsigned int i =0;i<commandInput.size();i++){
                                      if(commandInput.at(i)=='('){
                                          isNested =true;
                                      }
                                  }
                                  if(isNested)
                                  {
                                      vector<string> VecConnect;
                                      vector<string> chunksVec;
                                      
                                      unsigned begin;
                                      unsigned end;
                                      string chuncksPush;
                                      
                                      for(unsigned int i =0;i < commandInput.size();){
                                          if(commandInput.at(i)=='('){
                                              begin =i;
                                              end = perEnds(commandInput, i);
                                              chuncksPush = commandInput.substr(begin,end -begin+1);
                                              chunksVec.push_back(chuncksPush);
                                              i += end - begin+1;
                                          }
                                          else if(commandInput.at(i)=='&'){
                                              if(commandInput.at(i+1)=='&'){
                                                  VecConnect.push_back("&&");
                                              }
                                              i+=2;
                                          }
                                          else if(commandInput.at(i)==';'){
                                              
                                              VecConnect.push_back(";");
                                              
                                              i++;
                                          }
                                          else if(commandInput.at(i)==' '){
                                              
                                              
                                              i++;
                                          }
                                          else if(commandInput.at(i)=='|'){
                                              if(commandInput.at(i+1)=='|'){
                                                  VecConnect.push_back("||");
                                              }
                                              i+=2;
                                          }
                                          else{
                                              begin =i;
                                              unsigned a;
                                              unsigned b;
                                              unsigned c;
                                              c = commandInput.find(";",i);
                                              a = commandInput.find("&&",i);
                                              b = commandInput.find("||",i);
                                              
                                              if(commandInput.find("&&", i) == string::npos&&commandInput.find("||",i) ==string::npos && commandInput.find(";",i)== string::npos){
                                                  end= commandInput.size();
                                              }
                                              else{
                                                  if(a<b && a<c){
                                                      end = a-1;
                                                  }
                                                  else if(b< a && b<c){
                                                      end = b-1;
                                                  }
                                                  else if(c< a && c<b){
                                                      end = c-1;
                                                  }
                                              }
                                              chuncksPush = commandInput.substr(begin,end-begin);
                                              i+= end - begin;
                                              chunksVec.push_back(chuncksPush);
                                              
                                              
                                          }
                                      }
                                      
                                      
                                      
                                      Base* firstChunck = new Chunks(chunksVec[0]);
                                      bool boolean = firstChunck->evaluate();
                                      track.push_back(boolean);
                                      
                                      for(unsigned int j =0; j < VecConnect.size(); j++){
                                          Base* nextChunk;
                                          if(VecConnect[j]  == "&&"){
                                              nextChunk = new And(boolean, new Chunks(chunksVec[j+1]));
                                          }
                                          else if(VecConnect[j]  == "||"){
                                              nextChunk = new Or(boolean, new Chunks(chunksVec[j+1]));
                                          }
                                          else if(VecConnect[j]  == ";"){
                                              nextChunk = new Semicolon(boolean, new Chunks(chunksVec[j+1]));
                                          }
                                          bool nextC = nextChunk->evaluate();
                                          track.push_back(nextC);
                                          
                                      }
                                      for(unsigned int k=0; k< track.size();k++){
                                          if(track.at(k)==1){
                                              return 1;
                                          }
                                      }
                                      
                                      
                                      
                                   
                                  }
                                  else if(!isNested){
                                      int ind_one;
                                      int ind_two;
                                      
                                      if(commandInput.find('[') != string::npos){
                                          ind_one = commandInput.find('[');
                                          if(commandInput.find(']')){
                                              ind_two = commandInput.find(']');
                                          }
                                          else{
                                              cout << "Not closed"<< endl;
                                              exit(0);
                                              
                                          }
                                          commandInput.erase(ind_one,1);
                                          commandInput.erase(ind_two-1,1);
                                          commandInput.insert(0, "test ");
                                
                                      }
                                      
                                      vector<string> ConVec;
                                      for(unsigned l=0; l< commandInput.length();l++){
                                          if(commandInput[l]=='&'){
                                              if(commandInput[l+1]=='&'){
                                                  ConVec.push_back("&&");
                                              }
                                          }
                                          else if(commandInput[l]=='|'){
                                              if(commandInput[l+1]=='|'){
                                                  ConVec.push_back("||");
                                              }
                                          }
                                          else if(commandInput[l]==';'){
                                                  ConVec.push_back(";");
                                              
                                          }
                                      }
                                      
                                      vector<string> commands= parser(commandInput, "||&&;");
                                      vector<string> begincommands = parser(commands.at(0), " ");
                                      Base* first = new Command(begincommands);
                                      bool g = first->evaluate();
                                      track.push_back(g);
                                      
                                      
                                      for (unsigned i = 0; i < ConVec.size(); i ++) {
                                          Base* next;
                                          vector<string> args = parser(commands.at(i + 1), " ");
                                          if (ConVec.at(i) == "&&") {
                                              next = new And(g, new Command(args));
                                          }
                                          else if (ConVec.at(i) == "||") {
                                              next = new Or(g, new Command(args));
                                          }
                                          else if (ConVec.at(i) == ";") {
                                              next = new Semicolon(g, new Command(args));
                                          }
                                          bool cNext = next->evaluate();
                                          track.push_back(cNext);
                                      }
                                      
                                      for (unsigned int f = 0; f < track.size(); f++) {
                                          if (track[f] == 1) {
                                              return 1;
                                          }
                                      }
                                      return 0;
                                  }
                                  return 0;
                              }
                          };

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
            Base* line = new Chunks(inputCommand);
            line->evaluate();
            
            blank = true; //this means done with this command and wants next one
        }
    }
    return 0;
}
