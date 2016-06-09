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

using namespace std;

//forward declarations we needed
void exeCmd(string cmdLine, bool &executed);
bool hasSemicolon(string s);
bool hasHastag(string s);
bool isAConnector(string s);
bool chkRed(vector<string>str);
void stringParser(string cmdln, vector<string>&cmdArrray);
bool isTest(string str);

//execute function is the thing that will run al lthe functions
bool execute(vector<string>cmdVec) {
    //check to see if the input is exit then exit
	if (cmdVec.at(0) == "EXIT") {
		exit(0);
	}
	if(cmdVec.size() > 1){ //if command input is not exit
		if (cmdVec.at(1).at(0) == '\"'||cmdVec.at(1).at(0) ==  '\'') { //fixes quotation marks when executing echo
			cmdVec.at(1).erase(cmdVec.at(1).begin());
			cmdVec.at(cmdVec.size()-1).erase(cmdVec.at(cmdVec.size()-1).begin() + cmdVec.at(cmdVec.size()-1).size()-1);
		}
	}
	
	//here we will make an array of the commands so we can use execvp
	char* commandArray[cmdVec.size() + 1];
	//here we fill the array
	for (int i = 0; i < static_cast<int>(cmdVec.size()); i++) {
		commandArray[i] = (char*)cmdVec.at(i).c_str();	
	}
	//we will fork here and there are sys calls
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
			perror("EXECUTION FAILURE");
			exit(-1);
		}
	}
	else { //parent
		waitpid(pid, &pidstatus, 0); //wait till child is finished
	}	
	return true;
}
//this returns true or false depending on what happened to the test
bool testResult(vector<string>cmdVec){
    bool flag;
	struct stat file;
	//if there is a flag retur n true
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
	
//directory tag is -d
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
	
	else if(cmdVec.at(1) == "-f"){	//checks for file -f
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


//takes vctor and turns it into string this function is needed so we can do execvp
string toString(vector<string>cmd) {
	string str;
	for(unsigned i = 0; i < cmd.size(); i++) {
		str += cmd.at(i);
	}
	str += " ";
	return str;
}


//checks to see if there are pipes and then carries out the stuff needed to do
void pipeCmd(vector<string>cmd, vector<string>cmd2) {
	char* firstCommand[cmd.size() + 1]; //for the first command 
	for (int i = 0; i < static_cast<int>(cmd.size()); i++) { //vector -> character 
		firstCommand[i] = (char*)cmd.at(i).c_str();	
	}
	firstCommand[cmd.size()] = NULL; //we need last value null for system call
	

	char* secondCommand[cmd2.size() + 1]; //for the 2nd command 
	for (int i = 0; i < static_cast<int>(cmd2.size()); i++) { //make char array
		secondCommand[i] = (char*)cmd2.at(i).c_str();	
	}
	secondCommand[cmd2.size()] = NULL; 
				
  	int fileDesc[2]; //these are descritors for the files
  	pipe(fileDesc);
  	pid_t pid;

	
	if (fork() == 0) { // child process #1
		dup2(fileDesc[0], 0); // the std is changed to its descriptor
		close(fileDesc[1]); //here we need to close the pipe

		//Execute the second command
		execvp(secondCommand[0], secondCommand);
		perror("SYSCALL EXECVP FAILURE");// prints error message if execvp fails
	} 
	else if ((pid = fork()) == 0) { // child process #2
		dup2(fileDesc[1], 1); //change stdout to file output

		close(fileDesc[0]); //close the pipe, not going to read child process

		//Execute the first command
		execvp(firstCommand[0], firstCommand);
		perror("EXECVP DID NOT WORK");
	} 
	else { //parent process
		waitpid(pid, NULL, 0); //must wait until child is done
	}
}

//like the pipcmd but for redirect
bool executeRedirect(vector<string>cmdVec) { //will do the redirecting
//will take care of 3 different things <, >, >>
	int STDsv[2]; 
	vector<string>command;  // we need a vector of commands that are being redireted and loop wil lfill
	for(unsigned i = 0; i < cmdVec.size(); i++) {
		command.push_back(cmdVec.at(i));
		if((cmdVec.at(i) == ">") && command.size() > 1) { //redirection case 
		    //pop off redirect
			command.pop_back(); 
			char file[cmdVec.at(i+1).size()];
			strcpy(file,cmdVec.at(i+1).c_str());
			i++;
			
			int pFile;
			if((pFile = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP )) == -1) {
				perror("FILE OPEING FAILEd");
				exit(1);
			}
			STDsv[1] = dup(1); 
			dup2(pFile,1); 
			string commnd = toString(command);
			bool Executed = true;
			exeCmd(commnd,Executed);
			dup2(STDsv[1],1);
		}
		else if((cmdVec.at(i)==">>")) { //another redirection case 
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
		else if((cmdVec.at(i) == "<") && command.size() > 1) { //redirection case 
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
			dup2(STDsv[0],0); //this will restores stdn
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

//this executes command 
void exeCmd(string cmdLine, bool &executed) {
	vector<string>commandArray , command; 
	
	
	if (cmdLine == "EXIT") {
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
				if (executed) {
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
				if (!executed) { 
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
				if (!executed) {
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
				if (executed) {
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
bool isSingleCmd(string str){
    bool ret = true;
	for(int i = 0; i < str.size(); i++)
	    if(str.at(i) == ';');
	    ret = false;
	return ret;
}

bool isMultipleCmds(string str){
    bool ret = false;
    
	for(int i = 0; i < str.size(); i++)
	    if(str.at(i) == ';')
	    ret = true;
	return ret;
}
//this will check if the input is a connector
bool isAConnector(string str) {
    bool ret = false;
    vector<string> connectors;
    connectors.push_back("#");
    connectors.push_back("||");
    connectors.push_back("&&");
    connectors.push_back(";");
	for(int i = 0; i < connectors.size(); i++)
	    if(str == connectors.at(i))
	        ret = true;
	return ret;	
}
//checks if it is a test
bool isTest(string str){
	return (str == "test" || str == "[");
}
//checks if there is a comment
bool hasHastag(string str){
    bool ret = false;
	for(unsigned i = 0; i < str.size(); ++i)
			if(str.at(i) == '#') 
					ret = true;
	return ret;
}

//checks for ;
bool hasSemicolon(string str){ 
    bool ret = false;
	for(unsigned i = 0; i < str.size(); ++i){
		if(str.at(i) == ';'){
				ret = true; 
		}
	}
	return ret; 
}
//will parese the string and give out cmd array
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
				cmdArrray.push_back(";"); //pushes back command                   
			}
			else if(hasHastag(strToken)) { //take care of #
				strToken.erase(strToken.begin()); 
				cmdArrray.push_back("#"); //pushes back command
				cmdArrray.push_back(strToken);
			}
			else
				cmdArrray.push_back(strToken); 
		}
		else if(strToken.at(strToken.size() - 1) == ')'){//takes care of parenthesis
			if(strToken.size() == 1)
				cmdArrray.push_back(strToken);
			else{
				strToken.erase(strToken.begin() + strToken.size() -1 ); 
				cmdArrray.push_back(strToken); 
				cmdArrray.push_back(")");
			}
		}
		else if(strToken.at(0) == '('){
			if(strToken.size() == 1)
				cmdArrray.push_back(strToken);
			else{
				strToken.erase(strToken.begin());
				cmdArrray.push_back("(");  //takes careof ) and pushses it back
				cmdArrray.push_back(strToken); 
			}
		}
		else
			cmdArrray.push_back(strToken);
		token = strtok(NULL, " ");
	}
}  

//checks for redirection
bool chkRed(vector<string>str) {
    bool ret = false;
	for(unsigned i = 0; i < str.size(); i++)
		if (str.at(i) == "|" || str.at(i) == ">" || str.at(i) == ">>" || str.at(i) == "<")
			ret = true;
	return ret;
}
void display(vector<string> a){ //for error checking and testint code
    for(int i = 0; i < a.size(); i++){
        cout<<a.at(i)<<endl;
    }
}



//in main the whole thing runs
int main () {
	string cmdLine; //this is what the user will enter
	//for exiting
	while(cmdLine != "exit") {
		//getting username 
    	char* username = getlogin();
    	char hostname[BUFSIZ]; 
    	gethostname(hostname, 1024); //getting host name this is for Extra credit
    	if (username == NULL)
    		cout << "$ ";
    	else 
    		cout << "["<<username << '@' << hostname << "]$ ";
    		
		getline(cin, cmdLine);
		if (cmdLine == "exit")
			exit(0);
		else {
			bool Executed = true;
        	
        	vector<string>parseCommands;
        	vector<string>commandList;
        	stringParser(cmdLine,parseCommands);
        	string comm;
        	bool insideParenthesis = true;
        	//we need to check parn and parse
        	for(unsigned i = 0; i < parseCommands.size();i++) {
        		comm += parseCommands.at(i);
        		if(i < parseCommands.size()-1) { 
        			if(isAConnector(parseCommands.at(i+1)) && insideParenthesis) {
        				if(insideParenthesis){
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
        		if(i < parseCommands.size()-1 && !(parseCommands.at(i).size() - 1) == ')'){ 
        			comm += " ";
        		}
        
        		if(parseCommands.at(i).at(0) == '(') {
        			insideParenthesis = false;
        		}
        		else if(parseCommands.at(i).size() - 1 == ')') {
        		    //remove paren
        			for(unsigned i = 0; i < comm.size(); i++) {
    		                    if(comm.at(i) == '(' || comm.at(i) == ')') {
    		                	    comm.erase(comm.begin() + i);
    		                    }
    	                    }
        			commandList.push_back(comm);
        			comm.clear();
        			insideParenthesis = true;
        		}
        		else if(i == parseCommands.size() - 1) {
        		    //remove paren
        			for(unsigned i = 0; i < comm.size(); i++) {
    		                    if(comm.at(i) == '(' || comm.at(i) == ')') { //these segments are supposed to coreectly order the commandds
    		                	    comm.erase(comm.begin() + i);
    		                    }
    	                    }
        			commandList.push_back(comm);
        		}
        	}
        	//after removing all the parenthesis we can now enter in a list into execmd
        	for (unsigned i = 0; i < commandList.size(); i++) {
        		exeCmd(commandList.at(i),Executed);
        	}
		}
	}
	return 0;
}
