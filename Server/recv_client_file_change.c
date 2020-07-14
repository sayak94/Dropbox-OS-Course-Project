#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "send-recv.h"
#include "commands.h"
#include "recv_client_file_change.h"

void recv_file_change(int sock, char *dropboxPath, int commandIndex){

			if(commandIndex == file_created || commandIndex == sending_file){
	    		printf("File created:\n");
	    		recvfile(sock , dropboxPath);
			}
			else if(commandIndex == file_modified){
	    		printf("File modified:\n");
	    		recvfile(sock , dropboxPath);
			}
			else if(commandIndex == file_deleted){
	    		printf("File deleted:\n");
	    		delete_file_path(sock , dropboxPath);
			}
			else if(commandIndex == file_moved_from){
	    		printf("File moved from:\n");
	    		delete_file_path(sock , dropboxPath);
			}
			else if(commandIndex == file_moved_to){
	    		printf("File moved to:\n");
	    		recvfile(sock , dropboxPath);
			}
			else if(commandIndex == folder_created){
	    		printf("Folder created:\n");
	    		create_folder_path(sock , dropboxPath);
			}
			else if(commandIndex == folder_deleted){
	    		printf("Folder deleted:\n");
	    		delete_folder_path(sock , dropboxPath);
			}
}
