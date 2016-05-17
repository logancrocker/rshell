-----------------------
Introduction
-----------------------
rshell is a terminal, which has some of the functionality of any other terminal. It was programmed in C++.  
The command shell will print a command prompt (e.g. $) and read in a command on one line. More than one command 
may be input.  Once reading in commands the shell will parse the commands from the user and then execute them. 
Some of the many commands are ls, echo, mkdir, etc.

-----------------------
Known Bugs/Limitations
-----------------------
->The cd command does not work
->To use the OR and AND connectors, you must enter "||" or "&&" respectively. "|" or "&" will not work.

-----------------------
Installation
-----------------------
To run rshell, run the following commands:
git clone https://github.com/logancrocker/rshell.git
cd rshell
git checkout hw2
make
bin/rshell
