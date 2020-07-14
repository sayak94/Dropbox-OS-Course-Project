#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int getDirPath(char *fileName , char *dirPath , int wd){
    FILE *fp = fopen(fileName, "r");
    int mapFileWD;
    while(fscanf(fp ,"%d %[^\n]%*c" , &mapFileWD , dirPath) > 0){
        if(mapFileWD == wd){
            fclose(fp);
            return mapFileWD;
        }
    }
    fclose(fp);
    return -1;
}

int addDirPath(char *fileName , char *dirPath ,  int wd){
    FILE *fp = fopen(fileName, "a+");
    int ret = fprintf(fp, "%d %s\n", wd , dirPath);
    fclose(fp);
    return ret;
}

int *deleteDirPath(char *fileName , char *dirPathToBeDeleted){
    FILE *fp = fopen(fileName, "r");
    FILE *fpReplica = fopen("replica.txt" , "w+");
    char dirPath[200];
    int mapFileWD;
    int noOfFoldersRemoved = 0;
    int *retVal = (int *)malloc(noOfFoldersRemoved * sizeof(int));
    while(fscanf(fp ,"%d %[^\n]%*c" , &mapFileWD , dirPath) > 0){
        if((strlen(dirPath) >= strlen(dirPathToBeDeleted)) && (strstr(dirPath , dirPathToBeDeleted) != NULL)){
          noOfFoldersRemoved++;
          retVal = realloc(retVal , noOfFoldersRemoved);
          *(retVal + noOfFoldersRemoved - 1) = mapFileWD;
        }
        else{
          fprintf(fpReplica , "%d %s\n" , mapFileWD , dirPath);
        }
    }
    fclose(fp);
    fclose(fpReplica);
    remove(fileName);
    rename("replica.txt" , fileName);
    return retVal;
}
