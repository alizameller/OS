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
#include <dirent.h>
#include <grp.h>
#include <string.h>

void getArgs(int argc, char** argv, char** starting_path, int* uid, int* allusers, int* seconds) {
    *allusers = 1; //default setting to list inodes owned by anyone
    *seconds = 0;
    *uid = 0;
    *starting_path = argv[argc - 1];
    //printf("%s\n", *starting_path); // for debugging purposes
    char* username;
    int opt;
    while((opt = getopt(argc, argv, "u:m:")) != -1) {
        switch (opt) { 
            case 'u': 
                *allusers = 0; //change setting to list only inodes owned by user

                if (!(*uid = atoi(optarg))) { //if atoi returns 0 -> optarg = username
                    *uid = getpwnam(optarg)->pw_uid; 
                }
                //printf("%d\n", *uid); // for debugging purposes
                break;

            case 'm': 
                *seconds = abs(atoi(optarg));
                //printf("%d\n", *seconds); // for debugging purposes
                break;
        }
    }
}

void printMode(mode_t st_mode){
    mode_t filetype = st_mode & 0xF000; 
    char filec; 
    int mask1 = 0x0800;
    int uidBit = st_mode & mask1;
    int gidBit = st_mode & (mask1 >> 1);
    int stickyBit = st_mode & (mask1 >> 2);
    int mask2 = 0x01FF;
    mode_t perms = st_mode & mask2;

    switch(filetype) {
        case S_IFDIR:
            filec = 'd'; 
            break;
        case S_IFCHR:
            filec = 'c';
            break;
        case S_IFBLK:
            filec = 'b';
            break;
        case S_IFIFO:
            filec = 'p';
            break;
        case S_IFSOCK:
            filec = 's';
            break;
        case S_IFREG:
            filec = '-';
            break;
        case S_IFLNK:
            filec = 'l';
            break;
    }

    char permString[] = "rwxrwxrwx";

    if (uidBit) {
        permString[2] = 's';
    }
    if (gidBit) {
        permString[5] = 's';
    }
    if (stickyBit) {
        permString[8] = 't'; 
    }

    for (size_t i = 0; i < 9; i++) {
        permString[i] = (perms & (1 << (8-i))) ? permString[i] : '-';
    }

    printf("%c%s  ", filec, permString); 
}

void printName(uid_t st_uid, gid_t st_gid) {
    char *username;
    char *groupname;
    struct passwd *user;
    struct group *group;

    if (user = getpwuid(st_uid)) {
        username = user->pw_name;
    } else {
        username = (char *) st_uid; 
    }

    if (group = getgrgid(st_gid)) {
        groupname = group->gr_name;
    } else {
        groupname = (char *) st_gid;
    }

    printf("%s\t%s\t", username, groupname);
}

void printSize(mode_t st_mode, dev_t major, dev_t minor, off_t size) {
    mode_t filetype = st_mode & 0xF000; 
    if (filetype == S_IFBLK || filetype == S_IFCHR) {
        printf("%d, %d ", major, minor);
    } else {
        printf("%d ", size);
    }
}

// note: combine special and regular
void printInfo(struct stat *info, char *pathname) {
    printf("%d\t%d ", (int) info->st_ino, (int) info->st_blksize/1024);
    printMode(info->st_mode); 
    printf("%d  ", info->st_nlink);
    printName(info->st_uid, info->st_gid);
    printSize(info->st_mode, info->st_dev, info->st_rdev, info->st_size);
    char *time = ctime(&(info->st_mtimespec.tv_sec));
    printf("%s %s", time, pathname);
    if ((info->st_mode & 0xF000) == S_IFLNK) {
        char buf[150];
        ssize_t bytesRead = readlink(pathname, buf, 150);
        buf[bytesRead] = '\0';
        printf("-> %s", buf);
    }
    printf("\n");
}


//walks from one node to the next
void walk(char *path, struct stat *info) {
    if (lstat(path, info) == -1) { 
        //error
        return;
    }

   /* if (shouldPrint(path, info, ...)) {
        printInfo(info, path);
    }
    */

    if((info->st_mode & 0xF000) == S_IFDIR) {
        struct dirent *entry;
        DIR *directory;
        directory = opendir(path);
        // check if directory = NULL
        int length = strlen(path);
        // check if length = -1 (or error of some sort)
        while ((entry = readdir(directory)) != NULL) {
            if (*(entry->d_name) == '.' || !strcmp(entry->d_name, "..")) {
                continue;
            }
            char newPath[length + entry->d_namlen];
            strcpy(newPath, path);
            newPath[length] = '/';
            strcpy(newPath + length + 1, entry->d_name);
            /* for (int i = 0; i < entry->d_namlen; i++) {
                newPath[length + i] = entry->d_name[i]; 
            } */
            walk(newPath, info);
            printInfo(info, newPath);
        }
    }
}

int main(int argc, char *argv[]) {
    time_t modTime = time(NULL);
    struct stat info; 
    char* starting_path;
    int allusers;
    int uid;
    int seconds;

    getArgs(argc, argv, &starting_path, &uid, &allusers, &seconds);
    //lstat(starting_path, &info); 
    //printInfo(&info, starting_path); 
    walk(starting_path, &info);


    // To do:
    // 1. do walking (be able to get through dirs)
    // 2. write shouldPrint (whether a files info should be omitted or printed)
    // 3. error-handling/error-checking
    // 4. cleaning up
    // 5. extra credit! :) :)))))

    return 0;
}