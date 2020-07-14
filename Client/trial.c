#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "client_inotify_map.h"
#include "send_client_file_change.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN  ( 1024 * ( EVENT_SIZE + 16 ) )

int monitor(DIR *dir , char *dirPath , int inotifyFd , char *inotify_map_file){
    int inotifywd = inotify_add_watch( inotifyFd, dirPath , IN_ALL_EVENTS );
    if (inotifywd == -1){
        fprintf(stderr , "ERROR:Add watch failed.%d\n" , errno);
        return 0;
    }
    else {
        fprintf(stdout, "Watch added for %s\n", dirPath);
        addDirPath(inotify_map_file , dirPath , inotifywd);
    }
    struct dirent *ent;
    while((ent = readdir(dir)) != NULL){
        if(ent->d_name[0] != '.'){
            if(ent->d_type == DT_DIR){
                char *newDirPath = (char *)malloc((strlen(dirPath) + strlen(ent->d_name) + 3) * sizeof(char));
                strcpy(newDirPath , dirPath);
                strcat(newDirPath , "/");
                strcat(newDirPath , ent->d_name);
                strcat(newDirPath , "/\0");
                DIR *newDir = opendir(newDirPath);
                monitor(newDir , newDirPath , inotifyFd , inotify_map_file);
                free(newDirPath);
            }
        }
    }
    return 0;
}

int inotify_setup(char *dirPath , int clientsock){

    int inotifyFd = inotify_init();
    if (inotifyFd == -1){
        fprintf(stderr, "ERROR:inotify_init failed to initialise.\n");
        return -1;
     }
    DIR* dir = opendir(dirPath);
    char buffer[EVENT_BUF_LEN];
    char map_file[50] = "map.txt";
    FILE *fp = fopen(map_file , "w+");
    fclose(fp);
    if (dir){
        monitor(dir , dirPath , inotifyFd , map_file);
        closedir(dir);
    }
    else{
        fprintf(stderr , "ERROR:%s:%s\n" , strerror(errno) , dirPath);
        return 0;
    }

    while(1){
        int length = read( inotifyFd, buffer, EVENT_BUF_LEN );
        int i = 0;
        if (length < 0){
            fprintf(stderr, "ERROR:read failed.\n" );
        }
        while ( i < length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[i];
            if (event->len) {
                if (( event->mask & IN_CREATE)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was created at %s.\n", event->name, watchDirPath );
                            SendClientFolderCreated(clientsock , dirPath , watchDirPath , event->name);
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
			                  printf("%s\n" , watchDirPath);
			                  printf("%s\n" , event->name);
                        printf("%s\n" , newDirPath);
                        int inotifywd = inotify_add_watch( inotifyFd, newDirPath , IN_ALL_EVENTS );
                        if (inotifywd == -1){
                            fprintf(stderr , "ERROR:Add watch failed.%s\n" , strerror(errno));
                            return 0;
                        }
                        else {
                            fprintf(stdout, "Watch added for %s\n", event->name);
                            addDirPath(map_file , newDirPath , inotifywd);
                        }
                        free(watchDirPath);
                        free(newDirPath);
                    }
                    else{
                        if(event->name[0] != '.'){
                            char *watchDirPath;
                            watchDirPath = (char *)malloc(100 * sizeof(char));
                            if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                                printf( "The file %s was created at %s.\n", event->name, watchDirPath );
                                SendClientFileCreated(clientsock , dirPath , watchDirPath , event->name);
                            }
                            free(watchDirPath);
                        }
                    }
                }
                else if (( event->mask & IN_DELETE_SELF)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was deleted at %s.\n", event->name, watchDirPath );
                        }
                        inotify_rm_watch(inotifyFd , event->wd);
                        deleteDirPath(map_file , watchDirPath);
                        SendClientFolderDeleted(clientsock , dirPath , watchDirPath , event->name);
                        free(watchDirPath);
                    }
                }
                else if (( event->mask & IN_DELETE)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was deleted at %s.\n", event->name, watchDirPath );
                            SendClientFolderDeleted(clientsock , dirPath , watchDirPath , event->name);
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
                        int *wdToBeDeleted = deleteDirPath(map_file , newDirPath);
                        int noOfFoldersRemoved = sizeof(wdToBeDeleted)/sizeof(int);
                        int count = 0;
                        for (count = 0; count < noOfFoldersRemoved; count++){
                            if (*(wdToBeDeleted + count) != -1)
                                inotify_rm_watch(inotifyFd , *(wdToBeDeleted + count));
                        }
                        free(newDirPath);
                        free(watchDirPath);
                    }
                    else {
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The file %s was deleted at %s.\n", event->name, watchDirPath );
                            SendClientFileDeleted(clientsock , dirPath , watchDirPath , event->name);
                        }
                        free(watchDirPath);
                    }
                }
                else if (( event->mask & IN_CLOSE_WRITE)) {
                    if (event->mask & IN_ISDIR){
                        printf("%d\n" , event->mask);
                        /*char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was deleted at %s.\n", event->name, watchDirPath );
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
                        int wdToBeDeleted = deleteDirPath(map_file , newDirPath);
                        if (wdToBeDeleted != -1)
                            inotify_rm_watch(inotifyFd , wdToBeDeleted);
                        free(watchDirPath);*/
                    }
                    else {
                        if (event->name[0] != '.'){
                            char *watchDirPath;
                            watchDirPath = (char *)malloc(100 * sizeof(char));
                            if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                                printf( "The file %s was modified at %s.\n", event->name, watchDirPath );
                                SendClientFileModified(clientsock , dirPath , watchDirPath , event->name);
                            }
                            free(watchDirPath);
                        }
                    }
                }
                else if (( event->mask & IN_MOVE_SELF)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was moved from %s.\n", event->name, watchDirPath );
                            SendClientFolderDeleted(clientsock , dirPath , watchDirPath , event->name);
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
                        printf("%s\n" , newDirPath);
                        free(newDirPath);
                        free(watchDirPath);
                    }
                    else {
                        if (event->name[0] != '.'){
                            char *watchDirPath;
                            watchDirPath = (char *)malloc(100 * sizeof(char));
                            if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                                printf( "The file %s was moved from %s.\n", event->name, watchDirPath );
                                SendClientFileMovedFrom(clientsock , dirPath , watchDirPath , event->name);
                            }
                            free(watchDirPath);
                        }
                    }
                }
                else if (( event->mask & IN_MOVED_FROM)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was moved from %s.\n", event->name, watchDirPath );
                            SendClientFolderDeleted(clientsock , dirPath , watchDirPath , event->name);
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
                        int *wdToBeDeleted = deleteDirPath(map_file , newDirPath);
                        int noOfFoldersRemoved = sizeof(wdToBeDeleted)/sizeof(int);
                        int count = 0;
                        for (count = 0; count < noOfFoldersRemoved; count++){
                            if (*(wdToBeDeleted + count) != -1)
                                inotify_rm_watch(inotifyFd , *(wdToBeDeleted + count));
                        }
                        free(newDirPath);
                        free(watchDirPath);
                    }
                    else {
                        if (event->name[0] != '.'){
                            char *watchDirPath;
                            watchDirPath = (char *)malloc(100 * sizeof(char));
                            if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                                printf( "The file %s was moved from %s.\n", event->name, watchDirPath );
                                SendClientFileMovedFrom(clientsock , dirPath , watchDirPath , event->name);
                            }
                            free(watchDirPath);
                        }
                    }
                }
                else if (( event->mask & IN_MOVED_TO)) {
                    if (event->mask & IN_ISDIR){
                        char *watchDirPath;
                        watchDirPath = (char *)malloc(100 * sizeof(char));
                        if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                            printf( "The directory %s was moved to %s.\n", event->name, watchDirPath );
                            SendClientFolderCreated(clientsock , dirPath , watchDirPath , event->name);
                        }
                        char *newDirPath = (char *)malloc((strlen(watchDirPath) + strlen(event->name) + 4) * sizeof(char));
                        strcpy(newDirPath , watchDirPath);
                        strcat(newDirPath , "/");
                        strcat(newDirPath , event->name);
                        strcat(newDirPath , "/\0");
                        DIR *dir = opendir(newDirPath);
                        if (dir){
                            monitor(dir , newDirPath , inotifyFd , map_file);
                            closedir(dir);
                        }
                        free(newDirPath);
                        free(watchDirPath);
                    }
                    else {
                        if (event->name[0] != '.'){
                            char *watchDirPath;
                            watchDirPath = (char *)malloc(100 * sizeof(char));
                            if(event->wd == getDirPath(map_file , watchDirPath , event->wd)){
                              printf( "The file %s was moved to %s.\n", event->name, watchDirPath );
                              SendClientFileMovedTo(clientsock , dirPath , watchDirPath , event->name);
                            }
                            free(watchDirPath);
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
            else {
                i += EVENT_SIZE;
            }
        }
    }
  return 0;
}
