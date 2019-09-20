// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <string.h>
#include <sys/socket.h>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount(int sock) {
  bfs.mount();
  curr_dir = 1; //by default current directory is home directory, in disk block #1
  fs_sock = sock; //use this socket to receive file system operations from the client and send back response messages
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
  close(fs_sock);
}

// make a directory
void FileSys::mkdir(const char *name)
{
	if(strlen(name)>MAX_FNAME_SIZE){
		cout << "FIle name is too long" << endl;
		send_mes("504 File name is too long\r\n");
        send_mes("Length:0\r\n");
		
	}
	struct dirblock_t current_block;
	struct dirblock_t entry_dir_block;
	bfs.read_block(curr_dir, (void*) &current_block);
	if(current_block.num_entries == MAX_DIR_ENTRIES){
		cout << "Directory full"<<endl;
        send_mes("506 Directory is full");
        send_mes("Length:0\r\n");
		
	}

	int number_of_entries = 0;
	bool directoryExists = false;
	while(number_of_entries < current_block.num_entries){

		bfs.read_block(current_block.dir_entries[number_of_entries].block_num, (void*)&entry_dir_block);
		
		if (!is_directory(entry_dir_block.magic))
		{
			
			if(strcmp(current_block.dir_entries[number_of_entries].name,name) == 0){
				directoryExists = true;
             
			}
		}
		number_of_entries++;
	}

	if(!directoryExists){
		short blockSpace = bfs.get_free_block();
		if(blockSpace == 0){
			cout << "Disk Full"<<endl;
			send_mes("505 Disk is full");
			send_mes("Length:0\r\n");
		}
		struct dirblock_t new_directory;
		new_directory.num_entries = 0;
		new_directory.magic =  DIR_MAGIC_NUM;
		for (int i = 0; i < MAX_DIR_ENTRIES; i++){
	      new_directory.dir_entries[i].block_num = 0;
	    }

	    strcpy(current_block.dir_entries[current_block.num_entries].name, name);

	    current_block.dir_entries[current_block.num_entries].block_num == blockSpace;
	    current_block.num_entries++;
	    bfs.write_block(blockSpace,(void*)&new_directory);
	    bfs.write_block(curr_dir,(void*)&current_block);
	
	    send_mes("200 OK\r\n");
	    send_mes("Length:0\r\n");
	}
	else{
		   send_mes("502 File exists\r\n");
           send_mes("Length:0\r\n");

	}

	


}

// switch to a directory
void FileSys::cd(const char *name)
{
	struct dirblock_t current_block;
	struct dirblock_t changed_dir;

	
	int number_of_entries = 0;
	int k_entry = 0;
	bool fileExists = false;
	bool directory = false;
	bfs.read_block(curr_dir, (void*)&current_block);
	while(number_of_entries < current_block.num_entries){
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&changed_dir);
		
		if (strcmp(current_block.dir_entries[number_of_entries].name,name)==0)
		{
			fileExists = true;
		if (!is_directory(changed_dir.magic))
		{
			k_entry = number_of_entries;
			directory = true;

		} 
	}
		number_of_entries++;
	}

	if (fileExists == false)
	{
		send_mes("503 FIle does not exist");
		send_mes("Length:0\r\n");
	}

	if (directory == false)
	{
		send_mes("500 File is not a directory");
		send_mes("Length:0\r\n");
	}
	else{
		curr_dir = current_block.dir_entries[k_entry].block_num;
		send_mes("200 OK\r\n");
    	send_mes("Length:0\r\n");
	}

	
}

// switch to home directory
void FileSys::home() {
	curr_dir = 1;
    send_mes("200 OK\r\n");
    send_mes("Length:0\r\n");
}

// remove a directory
void FileSys::rmdir(const char *name)
{
	struct dirblock_t block_to_remove;
	struct dirblock_t current_block;

	bfs.read_block(curr_dir,(void*)&current_block);
	
	int number_of_entries = 0;
	int k_entry = 0;
	int remove = 0;
	bool directory = false;
	while ( number_of_entries < current_block.num_entries) {
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&block_to_remove);
		if (strcmp(current_block.dir_entries[number_of_entries].name,name) == 0)
		{	
			if (is_directory(block_to_remove.magic))
			{
				if(is_empty(block_to_remove.num_entries)){
					remove = current_block.dir_entries[number_of_entries].block_num;
					current_block.dir_entries[number_of_entries].block_num = 0;
					k_entry = number_of_entries;
					directory = true;
				} else {
				send_mes("507 Directory is not empty\r\n");
				send_mes("Length:0\r\n");
				}

			} 
			else{
				send_mes("500 File is not a directory\r\n");
				send_mes("Length:0\r\n");
			}
		}
		number_of_entries++;
	}
	if(directory){
		if (k_entry == current_block.num_entries)
		{
			current_block.num_entries--;
		} else {
			for (int i = k_entry; i < current_block.num_entries; i++)
			{
				for (int j = 0; j < MAX_FNAME_SIZE + 1; j++)
				{
					strcpy(&current_block.dir_entries[i].name[j],&current_block.dir_entries[i+1].name[j]);
				} 

				current_block.dir_entries[current_block.num_entries].block_num = 0;
				current_block.num_entries--;
			}
			
			
		}
			bfs.write_block(curr_dir,(void*)&current_block);
			bfs.reclaim_block(remove);
			send_mes("200 OK\r\n");
	    	send_mes("Length:0\r\n");
	}
	else{
		send_mes("500 File does not exist\r\n");
		send_mes("Length:0\r\n");
	}
}

// list the contents of current directory
void FileSys::ls()
{
	struct dirblock_t current_block;
	struct dirblock_t designated_block;
	string msg = "";

	bfs.read_block(curr_dir, (void*)&current_block);
	for (int i = 0; i < current_block.num_entries; i++)
		{
			bfs.read_block(current_block.dir_entries[i].block_num,(void*)&designated_block);
			msg += string(current_block.dir_entries[i].name);
			msg += " ";
			if(is_directory(designated_block.magic)){
				msg += "/";

			}

	}
    send_mes("200 OK\r\n");
    send_mes(msg+"\r\n");
	send_mes("Length: " + to_string(msg.length()) + "\r\n");

}

// create an empty data file
void FileSys::create(const char *name)
{
	if (strlen(name) > MAX_FNAME_SIZE)
	{
		send_mes("504 File name is too long\r\n");
		send_mes("Length:0\r\n");
		
	}

	struct dirblock_t current_block;
	struct dirblock_t entry_dir_block;

	bfs.read_block(curr_dir, (void*)&current_block);
	if (current_block.num_entries == MAX_DIR_ENTRIES)
	{
		send_mes("506 Directory is full");
		send_mes("Length:0\r\n");

	}
	bool fileExists = false;
	int number_of_entries = 0;
	while(number_of_entries < current_block.num_entries){
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&entry_dir_block);
		if (!is_directory(entry_dir_block.magic))
		{
			if (strcmp(current_block.dir_entries[number_of_entries].name,name)==0)
			{
				fileExists = true;
				send_mes("502 File exists");
				send_mes("Length:0\r\n");
				
			}
		}
		number_of_entries++;
	}
	if (!fileExists){
		short blockSpace = bfs.get_free_block();
		if (blockSpace == 0)
		{
			send_mes("505 Disk is full");
			send_mes("Length:0\r\n");
			
		}

		struct inode_t dFile;
		dFile.magic = INODE_MAGIC_NUM;
		dFile.size =0;
		for (int i = 0; i < MAX_DATA_BLOCKS; i++)
			dFile.blocks[i] = 0;

		strcpy(current_block.dir_entries[current_block.num_entries].name,name);
		current_block.dir_entries[current_block.num_entries].block_num = blockSpace;
		current_block.num_entries++;
		bfs.write_block(blockSpace,(void*)&dFile);
		bfs.write_block(curr_dir,(void*)&current_block);
		send_mes("200 OK\r\n");
	    send_mes("Length:0\r\n");
	}
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
struct dirblock_t current_block;
    struct inode_t requested_file;
    struct inode_t file_append;
    bfs.read_block(curr_dir,(void*) &current_block);
    int foundIndex = 0; //Hold the block number we are looking for
    bool matchingFile = false;

    for(foundIndex= 0; foundIndex < current_block.num_entries; foundIndex++){
        bfs.read_block(current_block.dir_entries[foundIndex].block_num, (void *) &requested_file);
        if(strcmp(current_block.dir_entries[foundIndex].name,name)==0){
            if(is_directory(requested_file.magic)){
            	send_mes("200 OK\r\n");
            	send_mes("File is a directory");
	            send_mes("Length:0\r\n");
            }
            else{
                matchingFile = true;
            }
        }
    }
    
    if(!matchingFile){
    	send_mes("200 OK\r\n");
        send_mes("File does not exist");
	    send_mes("Length:0\r\n");
  
    }
    else{

	    bfs.read_block(current_block.dir_entries[foundIndex].block_num, (void *) &file_append);
	    
	    if((file_append.size + strlen(data))>MAX_FILE_SIZE){
	        send_mes("200 OK\r\n");
	        send_mes("append exceeds maximum file size");
		    send_mes("Length:0\r\n");
	    }
	    else{
	    //FInd a non occupied block
	    bool freeSpot = false;
	    int freeSpotindex = 0;
	    int block = 0;
	    for(freeSpotindex = 0 ; (freeSpotindex < MAX_DATA_BLOCKS)&&(freeSpot = false) ; freeSpotindex++){
	        if(file_append.blocks[freeSpotindex] == 0){
	            freeSpot = true;
	            block = file_append.blocks[freeSpotindex];
	        }
	    }
	    
	    
	    // If the first block is Free
	    if(freeSpotindex == 0){
	        int appendedSoFar = 0;
	        int dataToAppend = strlen(data);
	        int requiredNumberOfBLocks = ceil(double(dataToAppend)/BLOCK_SIZE);

	        bool blockFull = false;
	        bool inBlock = false;
	        for(int i =0; i < requiredNumberOfBLocks; i ++){
	            struct datablock_t newData;
	            inBlock = false;
	            for(int j = 0; (j < BLOCK_SIZE) && (blockFull!=true); j++){

	                if(file_append.size==MAX_FILE_SIZE){
	                    blockFull = true;
	                }
	                if(dataToAppend>0){
	                    inBlock = true;
	                    newData.data[j] = data[appendedSoFar];
	                    appendedSoFar++;
	                    file_append.size++;
	                    dataToAppend--;
	                }
	            }
	            if(inBlock){
	                short freeBlock = bfs.get_free_block();
	                if(freeBlock == 0){

	                     send_mes("200 OK\r\n");
				        send_mes("Disk is full");
					    send_mes("Length:0\r\n");
	               
	                }else{
	                	  file_append.blocks[freeSpotindex] = freeBlock;
	                bfs.write_block(block, (void *) &newData);
	                bfs.write_block(current_block.dir_entries[foundIndex].block_num, (void *) &file_append);
	                  send_mes("200 OK\r\n");
					    send_mes("Length:0\r\n");
	                }
	              
	            }
	            if(blockFull){
	                send_mes("200 OK\r\n");
			        send_mes("append exceeds maximum file size\r\n");
				    send_mes("Length:0\r\n");
	            }
	        }
	    }
	    else{ // If the first block is not Free
		        struct datablock_t dataBlock;
		        bfs.read_block(block, (void *) &dataBlock);
		        int spaceLeft = (file_append.size - (BLOCK_SIZE*freeSpotindex));
		        if((spaceLeft + strlen(data)) <= BLOCK_SIZE){ // If all the data can fit in a block
		            for(int i =0; i < strlen(data); i++){
		                if(file_append.size != MAX_FILE_SIZE){
		                    dataBlock.data[spaceLeft] = data[i];
		                    file_append.size ++;
		                    spaceLeft ++;
		                }
		            }
		            bfs.write_block(block, (void *) &dataBlock);
		            bfs.write_block(current_block.dir_entries[foundIndex].block_num, (void *) &file_append);
		            if((file_append.size + strlen(data) ) > MAX_FILE_SIZE)
		            {
		                send_mes("200 OK\r\n");
				        send_mes("append exceeds maximum file size\r\n");
					    send_mes("Length:0\r\n");
		            }
		        }
		        else{ // If not all the data can fit in a block
		            
		            //Write and check the remaining data
		            int appendedSoFar = 0;
		            if(spaceLeft < BLOCK_SIZE){
		                for(int i =0; (i < BLOCK_SIZE)&&(file_append.size != MAX_FILE_SIZE); i++){
		                    if(spaceLeft < BLOCK_SIZE){
		                        dataBlock.data[spaceLeft] = data[i];
		                        spaceLeft ++;
		                        file_append.size ++;
		                        appendedSoFar++;
		                    }
		                }
		                bfs.write_block(block, (void *) &dataBlock);
		                bfs.write_block(current_block.dir_entries[foundIndex].block_num, (void *) &file_append);
		            }
		            if(file_append.size == MAX_FILE_SIZE){
		                send_mes("200 OK\r\n");
				        send_mes("append exceeds maximum file size\r\n");
					    send_mes("Length:0\r\n");
		            }
		            int blocksNeeded = ceil(double(strlen(data)-appendedSoFar)/BLOCK_SIZE);
		            int spaceConsumed = strlen(data) - appendedSoFar;
		            while(blocksNeeded!=0){
		                struct datablock_t dataAppend;
		                int additionalSpace = 0;
		                for(int i =0; (i < BLOCK_SIZE)&&(file_append.size != MAX_FILE_SIZE); i++){
		                    if(additionalSpace < spaceConsumed){
		                        dataAppend.data[spaceLeft] = data[appendedSoFar];
		                        additionalSpace ++;
		                        file_append.size ++;
		                        appendedSoFar++;
		                    }
		                }
		                spaceConsumed -= additionalSpace;
		                short freeSpot = bfs.get_free_block();
		                if(freeSpot == 0){
			                send_mes("200 OK\r\n");
					        send_mes("Disk is full");
						    send_mes("Length:0\r\n");
		                }
		                blocksNeeded--;
		                file_append.blocks[freeSpotindex] = freeSpot;
		                bfs.write_block(freeSpot, (void *) &dataAppend);
		                bfs.write_block(current_block.dir_entries[foundIndex].block_num, (void *) &file_append);
		                if(file_append.size == MAX_FILE_SIZE){
		                    send_mes("200 OK\r\n");
					        send_mes("append exceeds maximum file size\r\n");
						    send_mes("Length:0\r\n");
		                }
		            }
		            
		            
		        }
		    }

	    }
	   

    }
    
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
	struct inode_t appendFile;
	struct inode_t checkFile;
	struct dirblock_t current_block;
	int blockNum = 0;
	int number_of_entries = 0;
	bool fileExists = false;
	bool directory = true;
	string msg = "";
	bfs.read_block(curr_dir, (void*)&current_block);
	while(number_of_entries < current_block.num_entries){
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&checkFile);
		if (strcmp(current_block.dir_entries[number_of_entries].name,name) == 0)
		{
			fileExists = true;
			if (!is_directory(checkFile.magic))
			{
				directory = false;
				bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*) &appendFile);
				blockNum = number_of_entries;
			}
		}
		number_of_entries++;
	}

	if (fileExists == false)
	{
		send_mes("503 File does not exist");
		send_mes("Length:0\r\n");
	}

	if (directory)
	{
		send_mes("501 File is a directory");
		send_mes("Length:0\r\n");
	}
	if(!directory && fileExists){
		if (appendFile.size != 0)
		{
			int remain_data = appendFile.size;
			int cell = ceil(double(appendFile.size)/BLOCK_SIZE);

			for (int i = 0; i < cell; i++)
			{
				struct datablock_t data_block;
				bfs.read_block(appendFile.blocks[i],(void*)&data_block);
				if (i != (cell-1))
				{
					for (int j = 0; i < BLOCK_SIZE; j++)
					{
						msg += data_block.data[j];
						remain_data--;
					}
				} else {
					for (int j = 0; j < remain_data; j++)
					{
						msg += data_block.data[j];
					}
				}
			}
			cout << endl;
		}
		send_mes("200 OK\r\n");
		send_mes("Length: " + to_string(msg.length()) + "\r\n");
		send_mes("\r\n");
    	send_mes(msg+"\r\n");
		

	}
}

// display the first N bytes of the file
void FileSys::head(const char *name, unsigned int n)
{
}

// delete a data file
void FileSys::rm(const char *name)
{
	struct inode_t block_to_remove;
	struct dirblock_t current_block;

	bfs.read_block(curr_dir, (void*)&current_block);

	int number_of_entries = 0;
	bool fileExists = false;
	bool directory = true;
	int k_entry = 0;
	int remove = 0;
	int removeDat = 0;

	while (number_of_entries < current_block.num_entries) {
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&block_to_remove);
		if (strcmp(current_block.dir_entries[number_of_entries].name,name) == 0)
		{
			fileExists = true;
			if (!is_directory(block_to_remove.magic))
			{
				directory = false;
				remove = current_block.dir_entries[number_of_entries].block_num;
				k_entry = number_of_entries;
			}
		}
		number_of_entries++;
	}

	if (!fileExists)
	{
		send_mes("502 File exists");
		send_mes("Length:0\r\n");
	}
	else{

		if (directory)
		{
			send_mes("501 File is a directory");
			send_mes("Length:0\r\n");
		}
        else{


			if (k_entry == current_block.num_entries)
			{
				
				bfs.read_block(current_block.dir_entries[k_entry].block_num,(void*)&block_to_remove);
				for (int i = 0; i < MAX_DATA_BLOCKS; i++)
				{
					if (block_to_remove.blocks[i] != 0)
					{
						removeDat = block_to_remove.blocks[i];
						block_to_remove.blocks[i] = 0;
						bfs.reclaim_block(removeDat);
					}
				}
				bfs.write_block(current_block.dir_entries[k_entry].block_num,(void*)&block_to_remove);
				current_block.dir_entries[k_entry].block_num = 0;
				current_block.num_entries--;
			} else {
				bfs.read_block(current_block.dir_entries[k_entry].block_num,(void*)&block_to_remove);
				for (int i = 0; i < MAX_DATA_BLOCKS; i++)
				{
					if(block_to_remove.blocks[i] != 0 ){
						removeDat = block_to_remove.blocks[i];
						block_to_remove.blocks[i] = 0;
						bfs.reclaim_block(removeDat);
					}
				}
					
				bfs.write_block(current_block.dir_entries[k_entry].block_num,(void*)&block_to_remove);

				for (int i = k_entry; i < current_block.num_entries; i++)
				{
					for (int j = 0; j < MAX_FNAME_SIZE + 1; j++)
					{

						strcpy(&current_block.dir_entries[i].name[j],&current_block.dir_entries[i+1].name[j]);

					}
					
					current_block.dir_entries[i].block_num = current_block.dir_entries[i+1].block_num;
				}
				current_block.dir_entries[current_block.num_entries].block_num = 0;
				current_block.num_entries--;
			}
			

        }

       	    
			bfs.write_block(curr_dir,(void*)&current_block);
			bfs.reclaim_block(remove);
			send_mes("200 OK\r\n");
			send_mes("Length:0\r\n");


	}

	
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
	struct inode_t appendFile;
	struct inode_t checkFile;
	struct dirblock_t current_block;
	int blockNum = 0;
	int number_of_entries = 0;
	bool fileExist = false;

	bfs.read_block(curr_dir, (void*)&current_block);

	while(number_of_entries < current_block.num_entries){
		bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&checkFile);
		if (strcmp(current_block.dir_entries[number_of_entries].name,name) == 0)
		{	
			fileExist = true;
			if (is_directory(checkFile.magic))
			{
				cout << "Directory name: " << 
				current_block.dir_entries[number_of_entries].name <<"/"<< endl;
				cout << "Directory block: " << 
				current_block.dir_entries[number_of_entries].block_num << endl;
			}

			else {
				int number_of_blocks = 1;
				bfs.read_block(current_block.dir_entries[number_of_entries].block_num,(void*)&appendFile);
				for (int i = 0; i < MAX_DATA_BLOCKS; i++)
				{
					if (appendFile.blocks[i] != 0)
						number_of_blocks++;
				}
				cout << "Inode block: " << 
				current_block.dir_entries[number_of_entries].block_num << endl;
				cout << "Bytes in file: " << appendFile.size << endl;
				cout << "Inode block: " << number_of_blocks<< endl;
			}


			
		}
		number_of_entries++;
	}

	if (fileExist == false)
	{
		send_mes("503 File does not exist\r\n");
		send_mes("Length: 0\r\n");
	}

}

// HELPER FUNCTIONS (optional)
bool FileSys::is_directory(int magicNumber){

	return magicNumber == DIR_MAGIC_NUM;

}

bool FileSys::is_empty(int numEntries)
{
    return numEntries == 0;
}

bool FileSys::send_mes(string s){
    int iter = 0;
    for(int i = 0; i < s.length();i++){
        char c = s[i];
        int num_byte_sent = 0;
        int checkError;
        while (num_byte_sent < sizeof(char)) {
        	
            checkError = send(fs_sock,(void*)&c,sizeof(char),0);
            if (checkError == -1) {
                cerr << "Failed to send message"<<endl;
            }
            num_byte_sent += checkError;
        }
    }

}
