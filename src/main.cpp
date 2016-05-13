//
//  AHDFH.cpp
//  CS100
//
//  Created by Ammar Rizvi on 5/12/16.
//  Copyright © 2016 Ammar Rizvi. All rights reserved.
//

#include "AHDFH.hpp"
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
//#include <boost/algorithm/string.hpp>

using namespace std;
//using namespace boost::algorithm;


// Virtual Base Class
class Base{
public:
    Base(){};
    // Virtual Function that will be inherited
    virtual bool evaluate() = 0;
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
