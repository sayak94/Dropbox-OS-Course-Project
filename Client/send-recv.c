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
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN  ( 1024 * ( EVENT_SIZE + 16 ) )
#define min(a,b) (a>b?b:a)

void _mkdir( char *dirpath) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dirpath);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
            tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
      if(*p == '/') {
          *p = 0;
          mkdir(tmp, S_IRWXU);
          *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

void send_file(int sock, char *localdirpath, char *dirPath, char *filename){
	char filedata[300];
	int n;
	long int size;
	char *newDirPath = (char *)malloc((strlen(localdirpath)+strlen(dirPath) + strlen(filename) + 100) * sizeof(char));
	strcpy(newDirPath , localdirpath);
  strcat(newDirPath , "/");
	strcat(newDirPath , dirPath);
	char length[10];

	snprintf(length,sizeof(length),"%zu",strlen(dirPath));
	write(sock,length,4);
  write(sock,dirPath,strlen(dirPath)+1);

	snprintf(length,sizeof(length),"%zu",strlen(filename));
	write(sock,length,4);
	write(sock,filename,strlen(filename)+1);
	strcat(newDirPath , "/");
	strcat(newDirPath , filename);


	FILE *f;
  f=fopen(newDirPath,"rb");
	if(f==NULL){
		printf("\nError\n");
	  exit(0);
	}
  fseek(f,0L,SEEK_END);
  size=ftell(f);
  fseek(f,SEEK_SET,0);

  snprintf(filedata,sizeof(filedata),"%lu",size);

  snprintf(length,sizeof(length),"%zu",strlen(filedata));
  write(sock,length,4);
  write(sock,filedata,strlen(filedata)+1);

  while (!feof(f)){
     n=fread(filedata,sizeof(char),300,f);
	   write(sock,filedata,n);
   }
	 free(newDirPath);
	 fclose(f);
}

void send_file_path(int sock, char *localdirpath, char *dirPath, char *filename){
	char *newDirPath = (char *)malloc((strlen(localdirpath)+strlen(dirPath) + strlen(filename) + 100) * sizeof(char));
	strcpy(newDirPath , localdirpath);
  strcat(newDirPath , "/");
	strcat(newDirPath , dirPath);
	char length[10];

	snprintf(length,sizeof(length),"%zu",strlen(dirPath));
	write(sock,length,4);
        write(sock,dirPath,strlen(dirPath)+1);

	snprintf(length,sizeof(length),"%zu",strlen(filename));
	write(sock,length,4);
	write(sock,filename,strlen(filename)+1);
}

void delete_file_path(int sock , char *dropboxPath){
  long int length;
  int rdret;
  char filename[100], dirpath[1024], len[5];
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , dirpath , length + 1);
  char *totalPath = (char *)malloc((strlen(dropboxPath) + strlen(dirpath) + strlen(filename) + 100)*sizeof(char));
  strcpy(totalPath , dropboxPath);
  strcat(totalPath , dirpath);

  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , filename , length + 1);
  filename[rdret] = 0;
  strcat(totalPath , "/");
  strcat(totalPath , filename);
  remove(totalPath);
  free(totalPath);
}

void delete_folder_path(int sock , char *dropboxPath){
  long int length;
  int rdret;
  char filename[100], dirpath[1024], len[5];
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , dirpath , length + 1);
  char *totalPath = (char *)malloc((strlen(dropboxPath) + strlen(dirpath) + strlen(filename) + 100)*sizeof(char));
  strcpy(totalPath , dropboxPath);
  strcat(totalPath , dirpath);

  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , filename , length + 1);
  filename[rdret] = 0;
  strcat(totalPath , "/");
  strcat(totalPath , filename);
  rmdir(totalPath);
  free(totalPath);
}

void create_folder_path(int sock , char *dropboxPath){
  long int length;
  int rdret;
  char filename[100], dirpath[1024], len[5];
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , dirpath , length + 1);
  char *totalPath = (char *)malloc((strlen(dropboxPath) + strlen(dirpath) + strlen(filename) + 100)*sizeof(char));
  strcpy(totalPath , dropboxPath);
  strcat(totalPath , dirpath);

  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , filename , length + 1);
  filename[rdret] = 0;
  strcat(totalPath , "/");
  strcat(totalPath , filename);
  _mkdir(totalPath);
  free(totalPath);
}

void recvfile(int sock , char *dropboxPath){
  long int length;
  int rdret;
  char filedata[300], filename[100], dirpath[1024], len[5];
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , dirpath , length + 1);
  char *totalPath = (char *)malloc((strlen(dropboxPath) + strlen(dirpath) + strlen(filename) + 100)*sizeof(char));
  strcpy(totalPath , dropboxPath);
  strcat(totalPath , dirpath);
  _mkdir(totalPath);
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , filename , length + 1);
  filename[rdret] = 0;
  printf("Receiving file %s\n" , filename);
  read(sock , len , 4);
  length = atoi(len);
  rdret = read(sock , filedata , length + 1);
  filedata[rdret] = 0;
  length = atol(filedata);

  FILE *fp;
  strcat(totalPath , "/");
  strcat(totalPath , filename);

  fp = fopen(totalPath, "wb" );

  if(fp == NULL){
  	printf("Error!");
    exit(1);
  }

  while (length > 0) {
    rdret = read(sock,filedata,min(300,length));
    length = length - rdret;
    fwrite(filedata , sizeof(char) , rdret , fp);
  }
  free(totalPath);
  fclose(fp);
}

void rm_dir( char *localdirpath, char *dirPath ){
    char *completeDirPath = (char *)malloc((strlen(localdirpath)+strlen(dirPath) + 30) * sizeof(char));
    strcpy(completeDirPath , localdirpath);
    strcat(completeDirPath , "/");
    strcat(completeDirPath , dirPath);
    DIR *dir= opendir (completeDirPath);

    struct dirent *ent;
    while((ent = readdir(dir)) != NULL){
        if(ent->d_name[0] != '.'){
            if(ent->d_type == DT_DIR){
              char *newDirPath = (char *)malloc((strlen(dirPath) + strlen(ent->d_name) + 30) * sizeof(char));
              strcpy(newDirPath , dirPath);
              strcat(newDirPath , "/");
              strcat(newDirPath , ent->d_name);
              rm_dir(localdirpath, newDirPath );
              free(newDirPath);
            }
      else if(ent->d_type == DT_REG){
        strcpy(completeDirPath , localdirpath);
        strcat(completeDirPath , "/");
        strcat(completeDirPath , dirPath);
        strcat(completeDirPath , "/");
        strcat(completeDirPath , ent->d_name);
        remove(completeDirPath);
	      }
       }
    }
    strcpy(completeDirPath , localdirpath);
    strcat(completeDirPath , "/");
    strcat(completeDirPath , dirPath);
    rmdir(completeDirPath);
    free(completeDirPath);

}

void send_dir(int sock, char *localdirpath, char *dirPath ){
    char command[2];

    char *completeDirPath = (char *)malloc((strlen(localdirpath)+strlen(dirPath) + 30) * sizeof(char));

    strcpy(completeDirPath , localdirpath);
    strcat(completeDirPath , "/");
    strcat(completeDirPath , dirPath);
    DIR *dir= opendir (completeDirPath);
    //printf("%s\n",completeDirPath);
    free(completeDirPath);
    struct dirent *ent;
    snprintf(command , 2 , "%d" , folder_created);
    write(sock , command , 2);
    send_file_path(sock , localdirpath , dirPath , "");
    while((ent = readdir(dir)) != NULL){
        if(ent->d_name[0] != '.'){
            if(ent->d_type == DT_DIR){
              char *newDirPath = (char *)malloc((strlen(dirPath) + strlen(ent->d_name) + 30) * sizeof(char));
	            strcpy(newDirPath , dirPath);
	            strcat(newDirPath , "/");
              strcat(newDirPath , ent->d_name);
              send_dir(sock, localdirpath, newDirPath );
              free(newDirPath);
            }
  	    else if(ent->d_type == DT_REG){
  	      snprintf(command , 2 , "%d", sending_file);
  	      write(sock, command, 2);
  	      send_file(sock, localdirpath, dirPath, ent->d_name);
  	    }
      }
    }

}
