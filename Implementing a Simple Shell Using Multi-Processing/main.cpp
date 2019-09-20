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


#define MAX_LINE 70
const int MAX_ARGS = 50;

bool parse_command(int argc, char** argv, char** cmd1, char** cmd2){
    bool is_pipe = false;
    int split = -1;
    
    for (int i =0; i < argc; i++) {
        if (strcmp(argv[i],"|") == 0){
            is_pipe = true;
            split = i;
        }
    }
    
    if (is_pipe == true){
        for (int i = 0; i < split; i++){
            cmd1[i] = argv[i];
        }
        
        int count = 0;
        for (int i = split+1; i<argc; i++){
            cmd2[count] = argv[i];
            count++;
        }
        
        cmd1[split] = NULL;
        cmd2[count] = NULL;
    }
    return is_pipe;
}
int read_args(char **argv){
    char * str;
    string x;
    int argc = 0;
    
    while (cin >> x) {
        str = new char[x.size()+1];
        strcpy(str, x.c_str());
        argv[argc] = str;
        
        argc++;
        
        //get the enter
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
void pipe_cmd(char** cmd1, char** cmd2) {
    int fds[2]; // file descriptors
    pipe(fds);
    pid_t pid;
    // child process #1
    if (fork() == 0) {
        // Reassign stdin to fds[0] end of pipe.
        dup2(fds[0], 0);
        
        // Not going to write in this child process, so we can close this end
        // of the pipe.
        close(fds[1]);
        
        // Execute the second command.
        execvp(cmd2[0], cmd2);
        cout << "Process " << getpid() << " exits with 0\n" << endl;
        perror("execvp failed");
        
        // child process #2
    } else if ((pid =fork())== 0) {
        // Reassign stdout to fds[1] end of pipe.
        dup2(fds[1], 1);
        
        // Not going to read in this child process, so we can close this end
        // of the pipe.
        close(fds[0]);
        
        // Execute the first command.
        execvp(cmd1[0], cmd1);
        perror("execvp failed");
        
        // parent process
    } else
        waitpid(pid, NULL, 0);
        printf("Process %d exits with 0\n",getpid());
        close(fds[1]);
    
}

int main() {

    char *argv[MAX_ARGS],*cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
    int argc;
    bool piping;
    
    while(1){
        cout << ("Lpham_shell-> ");
        argc = read_args(argv);
        piping = parse_command(argc,argv,cmd1,cmd2);
        if (piping == true){
            pipe_cmd(cmd1,cmd2);
        } else {
            exeCmd(argc, argv);
        }
        for (int i = 0; i < argc; i++){
            argv[i] = NULL;
        }
    }
    return 0;
}
