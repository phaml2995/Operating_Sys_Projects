#include <iostream>
#include <string.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include "FileSys.h"
using namespace std;

bool recv_mes(string &msg, int connection);

int main(int argc, char* argv[]) {
    //system("rm DISK");
	if (argc < 2) {
		cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    //networking part: create the socket and accept the client connection

    int sock,newsocket,byte_rcv; //change this line when necessary!
    struct sockaddr_in address;
    //char buf[1024];
    int addr_len = sizeof(address);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        cerr << "Failed to create socket!" << endl;
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sock,(sockaddr*)&address,sizeof(address)) == -1) {
        cerr << "bind failed" << endl;;
        return -1;
    }
    
    if(listen(sock, 3) < 0){
        cerr << "listen failed" << endl;
        return -1;
    }
    
    if ((newsocket = accept(sock,NULL,0))<0){
        cerr << "accept failed" << endl;
        return -1;
    }
    //mount the file system
    FileSys fs;
    fs.mount(newsocket); //assume that sock is the new socket created
                    //for a TCP connection between the client and the server.   
    cout << "Server and Client are connected" << endl;
    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the clinet
    //until the client closes the TCP connection.
    while (true) {
        
        string recv;
        if (!recv_mes(recv,newsocket))
        {
            break;
        }
        string msg = recv.substr(0,recv.length()-2);
        string command = msg.substr(0,msg.find(" "));
        if (command == "mkdir")
        {
            string dir = msg.substr(6);
            cout << dir<< endl;
            fs.mkdir(dir.c_str());
        }
        else if (command == "cd")
        {
            string dir = msg.substr(3);
            fs.cd(dir.c_str());
        }
         else if (command == "home"){
            fs.home();
        }
        else if (command == "rmdir"){
            string dir = msg.substr(6);
            fs.rmdir(dir.c_str());
        }
        else if (command == "ls"){
            fs.ls();
           // cout<<"RETURNED FROM LS";
        }
         else if (command == "create"){
            string dir = msg.substr(7);
            fs.create(dir.c_str());
        } else if (command == "cat"){
            string dir = msg.substr(4);
            fs.cat(dir.c_str());
        } else if (command == "rm"){
            string dir = msg.substr(3);
            fs.rm(dir.c_str());
        } else if (command == "append"){
            string dir = msg.substr(7);
            string file = dir.substr(0, dir.find(" "));
            string data = dir.substr(file.length()+1);
            cout << data << " " << file << endl;
            fs.append(file.c_str(),data.c_str());
        } else if (command == "head"){
            string dir = msg.substr(0,msg.find("\r\n"));
            string fileDat = dir.substr(4);
            string val = fileDat.substr(1,fileDat.find(" "));
            string fileNDat = fileDat.substr(val.length()+1);
            string file = fileNDat.substr(0,fileNDat.find(" "));
            string data = fileNDat.substr(file.length()+1);
            fs.head(file.c_str(), stoi(data));
        } else {
            string dir = msg.substr(5);
            fs.stat(dir.c_str());
        }
 }

    //close the listening socket
    close(sock);
    //unmout the file system
    fs.unmount();

    return 0;
}

bool recv_mes(string &msg,int connection)
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
        checkError = recv(connection, (void*) & c, sizeof(char), 0);
        if (checkError == -1)
        {
            return false;
        }
    
        num_bytes_recv += checkError;
    }
    if (c == '\n' && slash_end)
      {
        done = true;
      }
      slash_end = (c == '\r');
      vec.push_back(c);
  
  }
    msg = string(vec.begin(), vec.end());
    return true;
}
