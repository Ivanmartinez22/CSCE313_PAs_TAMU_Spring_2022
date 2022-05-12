#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Tokenizer.h"


// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;
vector<string> cdhistory;
vector<string> command_history;
vector<pid_t> pids;
int tmp_in=dup(0);
int tmp_out=dup(1);

// vector<string> split(string str, string delim) // unused was for testing only
// {
//     vector<string> _return;
//     int begin = 0;
//     int end = str.find(delim);
//     while (end != -1) {
//         _return.push_back(str.substr(begin, end - begin));
//         begin = end + delim.size();
//         end = str.find(delim, begin);
//     }
//     _return.push_back(str.substr(begin, end - begin));
//     return _return;
// }


string get_path()
{
   char buf[4096];
   return ( getcwd(buf, sizeof(buf)) ? string( buf ) : string("") );
} 



// void redirection_pipe(char *const * com1, char *const * com2){//for pipe commands only works with two for now 
// //unused only used in early development
//     //open pipe out side of function/loop change fd  
//     int fds[2];
//     if(pipe(fds) < 0){ // create pipe
//         perror("pipe");
//         exit(2);
//     }

//     pid_t pid1 = fork(); //dispatch first process 
//     if (pid1 < 0) {  // error check
//         perror("fork1");
//         exit(2);
//     }

//     if(pid1 == 0){ 
//         dup2(fds[1],1); // dup stdout into write side of pipe (fds[1])
//         close(fds[0]);
//         if (execvp(com1[0], com1) < 0) {  // error check
//             perror("execvp1");
//             exit(2);
//         }
//     }


//     pid_t pid2 = fork(); //dispatch second process 
//     if (pid2 < 0) {  // error check
//         perror("fork2");
//         exit(2);
//     }

//     if(pid2 == 0){
//         dup2(fds[0],0);// dup stdin from read side of pipe (fds[0])
//         close(fds[1]);
//         if (execvp(com2[0], com2) < 0) {  // error check
//             perror("execvp2");
//             exit(2);
//         }
//     }
//     //exiting parent 
//     //closing parent pipe 
//     close(fds[0]);
//     close(fds[1]);
//     int wstatus = 0;
//     waitpid(pid2, &wstatus, 0);
//     cout << "finished redirection " << endl;

// }



int main () {
    //cdhistory.push_back(".");
    for (;;) {
        dup2(tmp_in, 0);
        dup2(tmp_out, 1);
        // need date/time, username, and absolute path to current dir
        //starter prompt
        //cout << YELLOW << "Shell$" << NC << " ";
        time_t timer = time(0);
        tm* local_time = localtime(&timer);
        string month_str = "";
        string months[] = {"Jan", "Feb", "Mar","Apr","May", "Jun", "Jul","Aug","Sep","Oct","Nov","Dec"};
        cout << YELLOW << (months[local_time->tm_mon]) << " "  << local_time->tm_mday << " " << local_time->tm_hour<< ":" << local_time->tm_min << ":" << local_time->tm_sec << " "  << getenv("USER") << ":" << get_path() << "$" << NC << " ";//default prompt 
        //custom prompt //♚
        //cout << YELLOW <<"IvanShell " <<local_time->tm_hour<< ":" << local_time->tm_min << ":" << local_time->tm_sec << " " << (local_time->tm_mon +1)  << "/"  << local_time->tm_mday << "/"  << (local_time->tm_year+1900) << " " << getenv("USER") << ":" << RED << get_path() << YELLOW << "➣" << NC << "  ";
        // get user inputted command
        int size_pid = pids.size();
        for(int i = 0; i < size_pid; i++){
            int wstatus = 0;
            //implement way to track which pids are there and to remove inactive pids 
            //cout << size_pid << endl;
            if(waitpid(pids[i], &wstatus, WNOHANG) == pids[i]){
                //cout << pids[i] << " Killed";
                pids[i] = pids[size_pid-1];
                pids.pop_back();
                size_pid--;
            }
            
            //cout << "killed child " << endl;
        }
        
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }
        if (input == "prev") {  // print exit message and break out of infinite loop
            input = command_history.back();
        }
        
        

        // get tokenized commands from user input
        Tokenizer tknr(input);
        
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        command_history.push_back(input);
        

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        

        int size = tknr.commands.size();
        // cout <<"num of commands: " << size << endl;
        if(tknr.commands[0]->args[0] == "cd"){ // handle cd commands 
        //Address edge cases if any are thought of 
            string path = ".";
            if(tknr.commands[0]->args.size() == 1){
                path = "/home/";
            }
            else if(tknr.commands[0]->args.size() == 2){
                path = tknr.commands[0]->args[1];
                if(path == "-"){
                    if(cdhistory.size() != 0){
                        path = cdhistory[cdhistory.size()-1];
                    }
                    else{
                        path = ".";
                    }
                    
                    //cout << path << endl;
                }
                else{
                    cdhistory.push_back(get_path());
                }
                
            }
            chdir(path.c_str()); // possibly add error checking for bad path


        }
        else{// fork to create child  
        // execvp handling 
            
            
            // int return_;
            
            for(int j = 0; j < size; j++){// run single commands
                int fds[2];
                if(pipe(fds) < 0){ // create pipe
                    perror("pipe");
                    exit(2);
                }
                char** args = new char*[tknr.commands[j]->args.size() + 1]; //turns command args into char* array for execvp
                int size_args = tknr.commands[j]->args.size();
                for(int i = 0; i < size_args; i++){
                    // cout << tknr.commands[j]->args[i] << endl;
                    char* temp = const_cast<char *>(tknr.commands[j]->args[i].c_str());
                    args[i] = temp;
                }
                args[tknr.commands[j]->args.size()] = NULL;
                pid_t pid = fork();
                if (pid < 0) {  // error check
                    perror("fork");
                    exit(2);
                }
                if (pid == 0) {//child process
                    if((!tknr.commands[j]->hasInput()) && (tknr.commands[j]->hasOutput())){
                        //cout << "need to redirect from command to file >" << endl;
                        int fd = open(tknr.commands[j]->out_file.c_str(), O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                        dup2(fd,1);
                        close(fd);
                    }
                    else if((tknr.commands[j]->hasInput()) && (!tknr.commands[j]->hasOutput())){
                        //cout << "need to redirect from file to command <" << endl;
                        int fd = open(tknr.commands[j]->in_file.c_str(),O_RDONLY,S_IRUSR|S_IRGRP|S_IROTH);
                        dup2(fd,0);
                        close(fd);
                        
                    }
                    else if((tknr.commands[j]->hasInput()) && (tknr.commands[j]->hasOutput())){
                        //cout << "> <" << endl;
                        int infd, outfd;
                        // open input and output files
                        infd = open(tknr.commands[j]->in_file.c_str(), O_RDONLY);
                        outfd = open(tknr.commands[j]->out_file.c_str(),O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                        // replace standard input with input file
                        dup2(infd, 0);
                        // replace standard output with output file
                        dup2(outfd, 1);
                        // close unused file descriptors
                        close(infd);
                        close(outfd);
                    }

                    if( j < (size - 1)){
                        dup2(fds[1] , 1);
                        close(fds[0]);
                    }
                    if (execvp(args[0], args) < 0) {  // error check
                        perror("execvp");
                        exit(2);
                    }
                }//
                else{ // if parent, wait for child to finish 
                    if( (j == (size - 1)) && (!tknr.commands[j]->isBackground())){
                        int wstatus = 0;
                        waitpid(pid,&wstatus,0);


                    }
                    if(tknr.commands[j]->isBackground()){
                        pids.push_back(pid);
                    }
                    
                    dup2(fds[0],0);
                    close(fds[1]);
                }
                delete[] args;
            }
            //reset std in and out to default
            dup2(tmp_in, 0);
            dup2(tmp_out, 1);
        }
    }
}

