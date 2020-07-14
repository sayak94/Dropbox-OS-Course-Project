#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include "commands.h"
#include "send-recv.h"
#include "recv_client_file_change.h"


int main(int argc , char *argv[]){

      char dirPath[1024];
      char path[1];
      path[0]=0;
      DIR* dir = opendir(argv[1]);
      if (dir){
          closedir(dir);
          strcpy(dirPath , argv[1]);
      }
      else{
          fprintf(stderr , "ERROR:%s:%s\n" , strerror(errno) , argv[1]);
          return 0;
      }
      char hostname[1024], ip[15];
      hostname[1023] = '\0';
      gethostname(hostname, 1023);
      printf("Directory exists and can be added to watch.\nInitialising server at %s for directory %s\n", hostname , dirPath );
      printf("Setting up socket for client connection...\n");
      struct sockaddr_in clientaddr , serveraddr;
      socklen_t serversock , newserversock , clientsize;
      int commandIndex , numbytesClientName;
      char command[2];
      fflush(stdin);
      serversock=socket(AF_INET , SOCK_STREAM,0);
      bzero((char*)&serveraddr , sizeof(serveraddr));
      FILE *fp = fopen("ip.conf","r");
      fscanf(fp,"%s",ip);
      fclose(fp);
      serveraddr.sin_family = AF_INET;
      serveraddr.sin_port = 2000;
      serveraddr.sin_addr.s_addr = inet_addr(ip);
      bind(serversock , (struct sockaddr*)&serveraddr , sizeof(serveraddr));
      printf("Socket setup complete.Default port 1.Waiting for client connection...\n");
      listen(serversock , 5);

      while(1){
          clientsize = sizeof(clientaddr);
          newserversock = accept(serversock , (struct sockaddr*)&clientaddr , &clientsize);
          read(newserversock , command , 1);
          commandIndex = atoi(command);

          if (commandIndex == install){
              FILE *confFilefp = fopen("dropbox.conf" , "a+");
              if (NULL == confFilefp){
                  printf("dropbox.conf could not be opened\n");
                  continue;
              }
              fseek(confFilefp , 0L , SEEK_END);
              int confFileSize = ftell(confFilefp);
              int clientPortNum;
              if (confFileSize == 0){
                  clientPortNum = 2;
              }
              else {
                  clientPortNum = 2;
              }
              fseek(confFilefp , 0L , SEEK_END);
              read(newserversock , command , 1);
              numbytesClientName = (int)(*command);
	            char *clientName = (char *)malloc(numbytesClientName * sizeof(char));
              read(newserversock , clientName , numbytesClientName + 1);
              printf("Received new connection from %s\n" , clientName);
              fprintf(confFilefp , "%s %d\n" , clientName , clientPortNum);
              fclose(confFilefp);
	            send_dir(newserversock, dirPath , path);
	            snprintf(command , 2 , "%d", last_file_sent);
	            write(newserversock, command, strlen(command));
            }

	       while(1){
              int rdret = read(newserversock , command , 2);
		          if(rdret == 0)
			           break;
		          int commandIndex = atoi(command);
              while(commandIndex == 0){
			           rdret = read(newserversock, command, 2);
			           commandIndex = atoi(command);
		          }
		          recv_file_change(newserversock, dirPath, commandIndex);
          }

        }

      close(serversock);
      close(newserversock);
}
