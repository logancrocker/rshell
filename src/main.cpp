#include <iostream>
#include <vector>
#include <stack>
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h> //used for syscalls
#include <sys/wait.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <pwd.h>

using namespace std;

//forward declarations we needed
void exeCmd(string cmdLine, bool &executed);
bool hasSemicolon(string s);
bool hasHastag(string s);
bool isAConnector(string s);
bool chkRed(vector<string>str);
void stringParser(string cmdln, vector<string>&cmdArrray);
bool isTest(string str);

bool execute(vector<string>cmdVec) {
	if (cmdVec.at(0) == "exit") {
		exit(0);
	}
	if(cmdVec.size() > 1){
		if (cmdVec.at(1).at(0) == '\"'||cmdVec.at(1).at(0) ==  '\'') { //fixes quotation marks when executing echo
			cmdVec.at(1).erase(cmdVec.at(1).begin());
			cmdVec.at(cmdVec.size()-1).erase(cmdVec.at(cmdVec.size()-1).begin() + cmdVec.at(cmdVec.size()-1).size()-1);
		}
	}
	
	//here we will make an array of the commands so we can use execvp
	char* commandArray[cmdVec.size() + 1];
	
	for (int i = 0; i < static_cast<int>(cmdVec.size()); i++) {
		commandArray[i] = (char*)cmdVec.at(i).c_str();	
	}
	//e will fork here
	commandArray[cmdVec.size()] = NULL; 
	int pidstatus = 0;	
	bool pidstates;
	char path[20] = "/bin/"; 	
	strcat(path, cmdVec[0].c_str()); 
	pid_t pid = fork();
	if(pid==0) { 
		int exec = execvp(path,commandArray); 
		if (exec == -1) { 
			return false;
			perror("Execute");
			exit(-1);
		}
	}
	else { //parent
		waitpid(pid, &pidstatus, 0); //wait till child is finished
	}	
	return true;
}
bool testResult(vector<string>cmdVec){
    bool flag;
	struct stat file;
	
	if( cmdVec.at(1) != "-e" && cmdVec.at(1) != "-f" && cmdVec.at(1) != "-d"){
			flag = true;
	}else {
			flag = false;
	}
	//checks for test case whit no specs
	if( cmdVec.at(1) == "-e" || !flag ){
		if( flag && stat(cmdVec.at(2).c_str(), &file) == 0){
			return true;
		}else if (!flag && stat(cmdVec.at(1).c_str(), &file) ==0){
			return true;
		}else {
			return false;
		}
	}

	

	else if(cmdVec.at(1) == "-d"){ // checks for directory
		if(stat(cmdVec.at(2).c_str(), &file) == 0) {
			if(S_ISDIR(file.st_mode)){
				return true;
			}else{
			    return false;
			}
		}else{
			return false;
		}
	}
	
	else if(cmdVec.at(1) == "-f"){	//checks for file
		if(stat(cmdVec.at(2).c_str(), &file) == 0){
			if(S_ISREG(file.st_mode)){
				return true;
			}else{
				return false;
			}
		}else{
			return false;
		}
	}
	return false;
}


//takes vctor and turns it into string
string toString(vector<string>cmd) {
	string str;
	for(unsigned i = 0; i < cmd.size(); i++) {
		str += cmd.at(i);
	}
	str += " ";
	return str;
}



void pipeCmd(vector<string>cmd, vector<string>cmd2) {
	char* firstCommand[cmd.size() + 1];
	for (int i = 0; i < static_cast<int>(cmd.size()); i++) { //convert the vector into a char* array for execvp
		firstCommand[i] = (char*)cmd.at(i).c_str();	
	}
	firstCommand[cmd.size()] = NULL; //set last value to NULL for execvp

	char* secondCommand[cmd2.size() + 1];
	for (int i = 0; i < static_cast<int>(cmd2.size()); i++) { //convert the second command into a char* array for execvp
		secondCommand[i] = (char*)cmd2.at(i).c_str();	
	}
	secondCommand[cmd2.size()] = NULL; //set last value to NULL for execvp
				
  	int fileDesc[2]; //file descriptors
  	pipe(fileDesc);
  	pid_t pid;

	
	if (fork() == 0) { // child process #1
		dup2(fileDesc[0], 0); // change stdin to fileDesc[0]
		close(fileDesc[1]); //close the end of the pipe

		//Execute the second command
		execvp(secondCommand[0], secondCommand);
		perror("execvp failed");
	} 
	else if ((pid = fork()) == 0) { // child process #2
		dup2(fileDesc[1], 1); //change stdout to file output

		close(fileDesc[0]); //close the pipe, not going to read child process

		//Execute the first command
		execvp(firstCommand[0], firstCommand);
		perror("execvp failed");
	} 
	else { //parent process
		waitpid(pid, NULL, 0);
	}
}

bool executeRedirect(vector<string>cmdVec) {

	int STDsv[2]; 
	vector<string>command; 
	for(unsigned i = 0; i < cmdVec.size(); i++) {
		command.push_back(cmdVec.at(i));
		if((cmdVec.at(i) == ">") && command.size() > 1) {
		    //pop off redirect
			command.pop_back(); 
			char file[cmdVec.at(i+1).size()];
			strcpy(file,cmdVec.at(i+1).c_str());
			i++;
			
			int pFile;
			if((pFile = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP )) == -1) {
				perror("Couldn't open file");
				exit(1);
			}
			STDsv[1] = dup(1); 
			dup2(pFile,1); 
			string commnd = toString(command);
			bool Executed = true;
			exeCmd(commnd,Executed);
			dup2(STDsv[1],1);
		}
		else if((cmdVec.at(i)==">>")) {
		    if(command.size() > 1){
    			command.pop_back(); 
    			char file[cmdVec.at(i+1).size()];
    			strcpy(file,cmdVec.at(i+1).c_str());
    			i++;
    			int pFile;
    			if((pFile = open(file, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP )) == -1) {
    				perror("CANNOT OPEN FILE");
    				exit(1);
    			}
    			STDsv[1] = dup(1);
    			dup2(pFile,1); 
    			string commnd = toString(command);
    			bool Executed = true;
    			exeCmd(commnd,Executed);
    			dup2(STDsv[1],1); 
		    }
		}
		else if((cmdVec.at(i) == "<") && command.size() > 1) {
			command.pop_back();
			char file[cmdVec.at(i+1).size()];
			strcpy(file,cmdVec.at(i+1).c_str());
			i++;
			int pFile;
			if((pFile = open(file, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP )) == -1) {
				perror("PROBLEM OPEING FILE");
				exit(1);
			}
			STDsv[0] = dup(0); //saves stdin
			dup2(pFile,0); //changes to directory the file input
			string commnd = toString(command);
			bool Executed = true;
			exeCmd(commnd,Executed);
			dup2(STDsv[0],0); //restores stdn
		}
		else if((cmdVec.at(i) == "|") && command.size() > 1) {
			vector<string>secondCommand;
			command.pop_back();
			++i;
			for (unsigned j = i; j < cmdVec.size(); j++) {
				if(!(cmdVec.at(j) == ">>" || cmdVec.at(j) == ">" || cmdVec.at(j) == "|" || cmdVec.at(j) == "<")){
					secondCommand.push_back(cmdVec.at(j));
				}
			}
			pipeCmd(command,secondCommand);
		}
	}
	return true;
}


//next to fucntions check for parenthesis
bool endParen(string str){
	if(str.at(str.size() - 1) == ')')
		return true; 
	return false; 
}

bool openParen(string str){ 
	if(str.at(0) == '(')
		return true; 
	return false; 
}

//this executes command 
void exeCmd(string cmdLine, bool &executed) {
	vector<string>commandArray , command; 
	
	
	if (cmdLine == "exit") {
		exit(0);
	}
	stringParser(cmdLine, commandArray);
	bool RedirectReq = false;
	
	//LOTS OF REDUNDANT CODE HERE SO MOST DOES NOT NEED TO BE COMMENTED
	for (unsigned i = 0; i < commandArray.size(); i++) { //exectues if there are two commands around a connector
		command.push_back(commandArray.at(i));

	
		if(isAConnector(commandArray.at(i)) && command.size() > 1) { //checks for next commands
			RedirectReq = chkRed(command);
			if(command.at(1) == "#") { //handels comments
					break;
			}

			if (command.at(0)== ";") {
				
				command.erase(command.begin()); 
				command.pop_back(); 
				if(isTest(command.at(0)))
					executed = testResult(command);
				else if(RedirectReq) 
					executed = executeRedirect(command);
				else
					executed = execute(command);
				command.clear();
				i--;
			}

			else if(command.at(0) == "&&") {
				 
				command.erase(command.begin()); //remove connector from command
				command.pop_back(); //remove connector at the end
				if (executed == true) {
					if(isTest(command.at(0)))
						executed = testResult(command);
					
					else if(RedirectReq) 
						executed = executeRedirect(command);
					else
						executed = execute(command);
				}
				command.clear();
				i--;
			}
			else if(command.at(0) == "||") {
				
				command.erase(command.begin());
				command.pop_back(); 
				if (executed == false) { 
					if(isTest(command.at(0)))
						executed = testResult(command);
					else if(RedirectReq) 
						executed = executeRedirect(command);
					else
						executed = execute(command);
				}
				else 
					executed = false;
				command.clear();
				i--;
			}
			else if(command.at(0) == "#") {
                if(command.size() == 1)
				    break; 
			}
			else if(!isAConnector(command.at(0))){ 
				i--;
				command.pop_back(); 
				if(isTest(command.at(0)))
					executed = testResult(command);
				else if(RedirectReq) 
					executed = executeRedirect(command);
				else
					executed = execute(command);
				command.clear();
			}
			RedirectReq = false;
		}
		else if(i == commandArray.size()-1) { //cheks for last case
			RedirectReq = chkRed(command);
			if(isAConnector(command.at(command.size()-1))) { 
				command.pop_back();
				break;
			}
			if (command.size() != 0 && command.at(0) == "||") {
				command.erase(command.begin());
				if (executed == false) {
					if(isTest(command.at(0)))
						executed = testResult(command);
					else if(RedirectReq)
						executed = executeRedirect(command);
					else
						executed = execute(command);
				}
			}
			else if (command.size() != 0 && command.at(0) == "&&") {
				command.erase(command.begin()); //delete connector
				if (executed == true) {
					if(isTest(command.at(0)))
						executed = testResult(command);
					else if(RedirectReq) 
						executed = executeRedirect(command);
					else
						executed = execute(command);
				}
			}
			else if (command.size() != 0 && command.at(0) == ";") {
				command.erase(command.begin());
				if(isTest(command.at(0)))
					executed = testResult(command);
				else if(RedirectReq)
					executed = executeRedirect(command);
				else
					executed = execute(command);
				break;
			}
			else if (command.size() != 0 && command.at(0) == "#") {
				command.erase(command.begin());
				return;
			}
			else {
				if(isTest(command.at(0)))
					executed = testResult(command);
				else if(RedirectReq) 
					executed = executeRedirect(command);
				else
					executed = execute(command);
			}
		}
	}
}

bool isAConnector(string str) {
    bool ret = false;
	if (str == "#" || str== "||" || str == "&&" || str == ";") {
		ret = true;
	}
	return ret;	
}
bool isTest(string str){
    bool ret = false;
	if(str == "test" || str == "["){
			ret= true;
	}
	return ret;
}
bool hasHastag(string str){
	for(unsigned i = 0; i < str.size(); ++i){
			if(str.at(i) == '#') {
					return true;
			}
	}
	return false;
}
bool hasSemicolon(string str){ 
    bool ret = false;
	for(unsigned i = 0; i < str.size(); ++i){
		if(str.at(i) == ';'){
				ret = true; 
		}
	}
	return ret; 
}
void stringParser(string cmdln, vector<string>&cmdArrray) {
	//convert to char and then tokenize
	char* token; 
	char* cmd = new char[cmdln.length() + 1];
	strcpy(cmd, cmdln.c_str());
	token = strtok(cmd, " "); 
	for (int i = 0; token != NULL; i++) {

		if (token == NULL)
			break;
		string strToken = string(token);
		//takes care of ;
		if(!isAConnector(strToken)) { 
			if (hasSemicolon(strToken)) {
			
				strToken.erase(strToken.begin() + strToken.size() -1 ); 
				cmdArrray.push_back(strToken); 
				cmdArrray.push_back(";");
			}
			else if(hasHastag(strToken)) { //take care of #
				strToken.erase(strToken.begin()); 
				cmdArrray.push_back("#"); 
				cmdArrray.push_back(strToken);
			}
			else
				cmdArrray.push_back(strToken); 
		}
		else if(endParen(strToken)){//takes care of parenthesis
			if(strToken.size() == 1)
				cmdArrray.push_back(strToken);
			else{
				strToken.erase(strToken.begin() + strToken.size() -1 ); 
				cmdArrray.push_back(strToken); 
				cmdArrray.push_back(")");
			}
		}
		else if(openParen(strToken)){
			if(strToken.size() == 1)
				cmdArrray.push_back(strToken);
			else{
				strToken.erase(strToken.begin());
				cmdArrray.push_back("("); 
				cmdArrray.push_back(strToken); 
			}
		}
		else
			cmdArrray.push_back(strToken);
		token = strtok(NULL, " ");
	}
}  
bool chkRed(vector<string>str) {
	for(unsigned i = 0; i < str.size(); i++) {
		if (str.at(i) == "|" || str.at(i) == ">" || str.at(i) == ">>" || str.at(i) == "<"){
			return true;
		}
	}
	return false;
}

//in main the whole thing runs
int main () {
	string cmdLine;
	//for exiting
	while(cmdLine != "exit") {
		
    	char* username = getlogin();
    	char hostname[BUFSIZ]; 
    	gethostname(hostname, 1024);
    	if (username == NULL)
    		cout << "$ ";
    	else 
    		cout << username << '@' << hostname << "$ ";
    		
		getline(cin, cmdLine);
		if (cmdLine == "exit") { 
			exit(0);
		}
		else {
			bool Executed = true;
        	
        	vector<string>parseCommands;
        	vector<string>commandList;
        	stringParser(cmdLine,parseCommands);
        	string comm;
        	bool insideParenthesis = false;
        	for(unsigned i = 0; i < parseCommands.size();i++) {
        		comm += parseCommands.at(i);
        		if(i < parseCommands.size()-1) { 
        			if(isAConnector(parseCommands.at(i+1)) && !insideParenthesis) {
        				if(!insideParenthesis){
        				    //removie paren
            				for(unsigned i = 0; i < comm.size(); i++) {
    		                    if(comm.at(i) == '(' || comm.at(i) == ')') {
    		                	    comm.erase(comm.begin() + i);
    		                    }
    	                    }
        				}
        				commandList.push_back(comm);
        				comm.clear();
        			}
        		} //dont add a space to end of paren
        		if(i < parseCommands.size()-1 && !endParen(parseCommands.at(i))) { 
        			comm += " ";
        		}
        
        		if(openParen(parseCommands.at(i))) {
        			insideParenthesis = true;
        		}
        		else if(endParen(parseCommands.at(i))) {
        		    //remove paren
        			for(unsigned i = 0; i < comm.size(); i++) {
    		                    if(comm.at(i) == '(' || comm.at(i) == ')') {
    		                	    comm.erase(comm.begin() + i);
    		                    }
    	                    }
        			commandList.push_back(comm);
        			comm.clear();
        			insideParenthesis = false;
        		}
        		else if(i == parseCommands.size() - 1) {
        		    //remove paren
        			for(unsigned i = 0; i < comm.size(); i++) {
    		                    if(comm.at(i) == '(' || comm.at(i) == ')') {
    		                	    comm.erase(comm.begin() + i);
    		                    }
    	                    }
        			commandList.push_back(comm);
        		}
        	}
        	for (unsigned i = 0; i < commandList.size(); i++) {
        		exeCmd(commandList.at(i),Executed);
        	}
		}
	}
	return 0;
}
