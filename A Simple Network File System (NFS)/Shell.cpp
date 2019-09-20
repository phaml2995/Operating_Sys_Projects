// CPSC 3500: Shell
// Implements a basic shell (command line interface) for the file system

#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
using namespace std;

#include "Shell.h"

static const string PROMPT_STRING = "NFS> ";	// shell prompt

// Mount the network file system with server name and port number in the format of server:port
void Shell::mountNFS(string fs_loc) {
	//create the socket cs_sock and connect it to the server and port specified in fs_loc
	//if all the above operations are completed successfully, set is_mounted to true
  if (is_mounted)
  {
    cout << "Mounted" << endl;
    return;
  }

  string server,port;
  stringstream stream(fs_loc);
  getline(stream,server,':');
  getline(stream, port, '\0');
  addrinfo* address, hints;

  cout << "Server Name: " << server << " Port: " << port << endl;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(server.c_str(),port.c_str(),&hints,&address)!=0)
    {
      cerr << "Get addr failed" << endl;
      return;
    }


  if((cs_sock = socket(AF_INET,SOCK_STREAM,0)) == -1){
      cerr << "Failed to create socket" << endl;
      freeaddrinfo(address);
      return;
  }


  if (connect(cs_sock,address->ai_addr,address->ai_addrlen) == -1)
  {
    cerr << "Connect failed" << endl;
    freeaddrinfo(address);
    return;
  }

  //free dynamic memory
  freeaddrinfo(address); 
  is_mounted = true;
  cout << "Successfully connected" << endl;
  
}

// Unmount the network file system if it was mounted
void Shell::unmountNFS() {
	// close the socket if it was mounted
  close(cs_sock);
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname) {
  send_mes("mkdir " + dname + "\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;
}

// Remote procedure call on cd
void Shell::cd_rpc(string dname) {
  send_mes("cd " + dname + "\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;

}

// Remote procedure call on home
void Shell::home_rpc() {
  send_mes("home\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;

}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname) {
  send_mes("rmdir " + dname + "\r\n");
  string recv;
  recv_mes(recv);
  cout <<  recv << endl;
  recv_mes(recv);
  cout << recv << endl;
}

// Remote procedure call on ls
void Shell::ls_rpc() {
  send_mes("ls\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;
  // recv_mes(recv);
  // cout << recv << endl;
}

// Remote procedure call on create
void Shell::create_rpc(string fname) {
  send_mes("create " + fname + "\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  recv_mes(recv);
  cout << recv << endl;
}

// Remote procedure call on append
void Shell::append_rpc(string fname, string data) {
  send_mes("append " + fname + " " + data + "\r\n");
  string recv;
  recv_mes(recv);
  cout <<  recv << endl;
}

// Remote procesure call on cat
void Shell::cat_rpc(string fname) {
  send_mes("cat " + fname + "\r\n");
  string recv;
  recv_mes(recv);
  cout << recv << endl;
  if (recv == "200 OK \r\n")
  {
    recv_mes(recv);
    cout << recv << endl;
    recv_mes(recv);
    cout << recv << endl;
  }
  recv_mes(recv);
  cout << recv << endl;
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n) {
    send_mes("head " + fname + " " + to_string(n) + "\r\n");
    string recv;
    recv_mes(recv);
    cout << recv << endl;
    if (recv == "200 OK \r\n")
    {
        recv_mes(recv);
        cout << recv << endl;
        recv_mes(recv);
        cout << recv << endl;
    }
    recv_mes(recv);
    cout << recv << endl;
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname) {
    string recv;
    send_mes("rm " + fname + "\r\n");
    cout << "Calling rm" << endl;
    recv_mes(recv);
    cout << recv << endl;
    recv_mes(recv);
    cout << recv << endl;
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname) {
    send_mes("stat " + fname + "\r\n");
    string recv;
    recv_mes(recv);
    cout << recv << endl;
    if (recv == "200 OK \r\n")
    {
        recv_mes(recv);
        cout << recv << endl;
        recv_mes(recv);
        cout << recv << endl;
    }
    recv_mes(recv);
    cout << recv << endl;
}


// Executes the shell until the user quits.
void Shell::run()
{
  // make sure that the file system is mounted
  if (!is_mounted)
 	return; 
  
  // continue until the user quits
  bool user_quit = false;
  while (!user_quit) {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }

  // unmount the file system
  unmountNFS();
}

// Execute a script.
void Shell::run_script(char *file_name)
{
  // make sure that the file system is mounted
  if (!is_mounted)
  	return;
  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail()) {
    cerr << "Could not open script file" << endl;
    return;
  }


  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit) {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  unmountNFS();
  infile.close();
}


// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "") {
    return false;
  }
  else if (command.name == "mkdir") {
    mkdir_rpc(command.file_name);
  }
  else if (command.name == "cd") {
    cd_rpc(command.file_name);
  }
  else if (command.name == "home") {
    home_rpc();
  }
  else if (command.name == "rmdir") {
    rmdir_rpc(command.file_name);
  }
  else if (command.name == "ls") {
    ls_rpc();
  }
  else if (command.name == "create") {
    create_rpc(command.file_name);
  }
  else if (command.name == "append") {
    append_rpc(command.file_name, command.append_data);
  }
  else if (command.name == "cat") {
    cat_rpc(command.file_name);
  }
  else if (command.name == "head") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      head_rpc(command.file_name, n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "rm") {
    rm_rpc(command.file_name);
  }
  else if (command.name == "stat") {
    stat_rpc(command.file_name);
  }
  else if (command.name == "quit") {
    return true;
  }

  return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str)
{
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  if (ss >> command.name) {
    num_tokens++;
    if (ss >> command.file_name) {
      num_tokens++;
      if (ss >> command.append_data) {
        num_tokens++;
        string junk;
        if (ss >> junk) {
          num_tokens++;
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0) {
    return empty;
  }
    
  // Check for invalid command lines
  if (command.name == "ls" ||
      command.name == "home" ||
      command.name == "quit")
  {
    if (num_tokens != 1) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "mkdir" ||
      command.name == "cd"    ||
      command.name == "rmdir" ||
      command.name == "create"||
      command.name == "cat"   ||
      command.name == "rm"    ||
      command.name == "stat")
  {
    if (num_tokens != 2) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "append" || command.name == "head")
  {
    if (num_tokens != 3) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl; 
    return empty;
  } 

  return command;
}

bool Shell::send_mes(string s)
 {
   for (int i = 0; i < s.length(); i++)
   {
     char c = s[i];
     int num_bytes_sent = 0;
     int checkError;
     while (num_bytes_sent < sizeof(char))
     {
       checkError = send(cs_sock, (void*) & c, sizeof(char), 0);
       if (checkError == -1)
       {
         perror("Failed to send message");
       }
       num_bytes_sent += checkError;
     }
   }
 }



 bool Shell::recv_mes(string &msg)
{


  vector<char> vec;
  char c;
  bool done = false;
  bool slash_end = false;
  while(!done)
  {
    int num_bytes_recv = 0;
    int checkError;
    while (num_bytes_recv < sizeof(char))
    {
        checkError = recv(cs_sock, (void*) & c, sizeof(char), 0);
       // cout<<"RECEIVING SOMETHING END"<<c<<endl;
        if (checkError == -1)
        {
            return false;
        }
    
        num_bytes_recv += checkError;
    }
    if (c == '\n' && slash_end)
      {
      //  cout<<"THE n is REACHED "<<c<<endl;
        done = true;
      }
      if(c == '\r'){
 //cout<<"THE r is REACHED "<<c<<endl;
      }
      slash_end = (c == '\r');
      vec.push_back(c);
  
  }  


    msg = string(vec.begin(), vec.end());
    return true;
}

