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
#include "send_client_file_change.h"

void SendClientFileCreated(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", file_created);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFileModified(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", file_modified);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFileDeleted(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", file_deleted);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file_path(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFileMovedFrom(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", file_moved_from);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file_path(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFileMovedTo(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", file_moved_to);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFolderDeleted(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  snprintf(command , 2 , "%d", folder_deleted);
  write(clientsock,command,2);
  char *dirPath = watchDirPath + strlen(localdirPath);
  send_file_path(clientsock , localdirPath , dirPath , fileName);
}

void SendClientFolderCreated(int clientsock, char *localdirPath, char *watchDirPath, char *fileName){
  char command[2];
  char *dirPath = watchDirPath + strlen(localdirPath);
  char *totalDirPath = (char *)malloc((strlen(dirPath) + strlen(fileName) + 10)*sizeof(char));
  strcpy(totalDirPath , dirPath);
  strcat(totalDirPath , "/");
  strcat(totalDirPath , fileName);
  send_dir(clientsock , localdirPath , totalDirPath);
  snprintf(command , 2 , "%d", last_file_sent);
  write(clientsock, command, strlen(command));
  free(totalDirPath);
}
