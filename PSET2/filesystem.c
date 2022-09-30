#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <getopt.h>
#include <time.h>

void getArgs(int argc, char** argv, char** starting_path, int* uid, int* allusers, int* seconds) {
    *allusers = 1; //default setting to list inodes owned by anyone
    *seconds = 0;
    *uid = 0;
    *starting_path = argv[argc - 1];
    char* username;
    int opt;
    while((opt = getopt(argc, argv, "u:m:")) != -1) {
        switch (opt) { 
            case 'u': 
                *allusers = 0; //change setting to list only inodes owned by user

                if (!(*uid = atoi(optarg))) { //if atoi returns 0 -> optarg = username
                    *uid = getpwnam(optarg)->pw_uid; 
                }
                break;

            case 'm': 
                *seconds = atoi(abs(optarg));

                break;
        }
    }
}

void checkPrintu(struct stat info){
    return;
}

void doPrint(struct stat info) {
    printf("  inode:   %d\n",   (int) info.st_ino);
    printf(" dev id:   %d\n",   (int) info.st_dev);
    printf("   mode:   %08x\n",       info.st_mode);
    printf("  links:   %d\n",         info.st_nlink);
    printf("    uid:   %d\n",   (int) info.st_uid);
    printf("    gid:   %d\n",   (int) info.st_gid);
    return; 
}

//walks from one node to the next
void walk(char *path, struct stat *buf) {
    if (!lstat(path, buf)) {
    //error
    return;
    }
}


int main() {
    time_t modTime = time();
    struct stat info; 
    char* starting_path;
    int allusers;
    int uid;
    int seconds;
    char* username; 
    // call getArgs to get -u value -m value and starting_path
    // call walk 

    return 0;
}