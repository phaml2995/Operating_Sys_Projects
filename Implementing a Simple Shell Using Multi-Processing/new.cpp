//
//  main.cpp
//  Project1
//
//  Created by Long Pham on 1/17/19.
//  Copyright © 2019 Long Pham. All rights reserved.
//

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;


#define MAX_LINE 100
const int MAX_ARGS = 50;

int parse_command(int argc, char** argv, char *cmd[50][100]){
    
    int row = 0,col = 0;
    //int split = -1;
    
    for (int i =0; i < argc; i++) {
        if ((strcmp(argv[i],"|")) == 0){
            cmd[row][col] = NULL;
            col++;
            row = 0;
        } else {
            cmd[row][col] = argv[i];
            row++;
        }
    }
   
    return (col+1);
}

int read_args(char **argv){
    char *str;
    string x;
    int argc = 0;
    
    while (cin >> x) {
        str = new char[x.size()+1];
        strcpy(str, x.c_str());
        argv[argc] = str;
        
        argc++;
        
        if (cin.get() == '\n'){
            break;
        }
    }
    argv[argc] = NULL;
    return argc;
    
}

void exeCmd(int argc, char** argv){
    
    pid_t pid;
    pid = fork();
    
    if(pid < 0){
        perror("Cannot Fork!");
        exit(-1);
    } else if (pid == 0 ){
        execvp(argv[0], argv);
        perror("execvp failed");
    } else if (pid > 0){
        wait(NULL);
       
        printf("Process %d exits with 0\n",getpid());
        exit(0);
    }
    
}

void pipe_cmd(int numofCmd,char*cmd[50][100]) {
    int fd[2]; // file descriptors;
    int count = 0;
    
    while (count < numofCmd)
    {
        pid_t pid;
        pipe(fd);
        pid = fork();
        
        if (pid < 0){ // fork failed
            perror("Fork failed");
        }
        else if (pid != 0) { //Parent Process
            waitpid(pid,NULL,0);
            close(fd[1]);
            printf("Process %d exits with 0\n",getpid());
        }
        else{ //Child Process
            dup2(fd[0],0);
            if (count != numofCmd){
                dup2(fd[1], 1);
            }
            close(fd[0]);
            exit(execvp(cmd[count][0], cmd[count]));
            //perror("execvp failed");
            
        }
     
        count++;
    }
}

int main() {
    
    char *argv[MAX_ARGS],*cmd[MAX_ARGS][MAX_LINE];
    int argc;
    int piping;
    
    while(1){
        cout << ("Lpham_shell >> ");
        argc = read_args(argv);
        piping = parse_command(argc,argv,cmd);
        cout << piping << endl;
//        for (int i = 0; i < 2; ++i) {
//            for (int j = 0; i < 2; ++j) {
//                cout << cmd[i][j] << ' ';
//            }
//            cout << endl;
//        }
       // pipe_cmd(piping, cmd);
        if (piping > 1){
            pipe_cmd(piping, cmd);
        }else {
            exeCmd(argc,argv);
        }

//        for (int i = 0; i < argc; i++){
//            argv[i] = NULL;
//        }
    }
    return 0;
}

