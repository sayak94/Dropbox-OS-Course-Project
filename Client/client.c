#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "commands.h"
#include "send-recv.h"
#include "trial.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN  ( 1024 * ( EVENT_SIZE + 16 ) )

int main(){
    printf("Enter path of the directory where to synchronise:");
    char dirPath[1024];
    bool isValidDirPath = false;
    while(!isValidDirPath){
        scanf("%s" , dirPath);
        printf("%s\n" , dirPath);
        DIR *dir = opendir(dirPath);
        if(dir){
          printf("Directory already exists. Replace?(y or n):");

          char c = getchar();
          c = getchar();
          if (c == 'y' || c == 'Y'){
              isValidDirPath = true;
              rm_dir(dirPath , "");
          }
          else {
              printf("Enter path of the directory where to synchronise:");
          }
        }
        else {
          if (errno == 2){
              isValidDirPath = true;
          }
          else {
              printf("Error:Enter a different path:");
          }
      }
    }
    FILE *fp = fopen("ip.conf" , "r");
    char ip[16];
    fscanf(fp , "%s" , ip);
    fclose(fp);
    mkdir(dirPath, 0700);
    struct sockaddr_in serveraddr;
    int clientsock,length;
    char command[20];
    bzero((char*)&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=2000;
    serveraddr.sin_addr.s_addr=inet_addr(ip);
    clientsock=socket(AF_INET,SOCK_STREAM,0);
    if(connect(clientsock,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0){
        printf("Error:Cannot connect...\n");
        exit(0);
    }
    length = snprintf(command , 2 , "%d", install);
    write(clientsock,command,length);
    char hostName[1024];
    hostName[1023]= '\0';
    gethostname(hostName,1023);
    length = strlen(hostName);
    write(clientsock,(char *)&length,1);
    write(clientsock,hostName,length + 1);

    read(clientsock,command,2);
    int commandNum = atoi(command);

    while (commandNum == sending_file || commandNum == folder_created){
      if (commandNum == sending_file){
        recvfile(clientsock , dirPath);
        read(clientsock,command,2);
        commandNum = atoi(command);
      }
      else {
    	  create_folder_path(clientsock , dirPath);
        read(clientsock,command,2);
        commandNum = atoi(command);
      }
    }

    if (commandNum == 3){
      fprintf(stdout , "All files received successfully.\n");
    }

    inotify_setup(dirPath , clientsock);
    return 0;
}
