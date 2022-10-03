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
#include <limits.h>


void getArgs(int argc, char** argv, char** starting_path, int* uid, int* allusers, int* seconds) {
    *allusers = 1; //default setting to list inodes owned by anyone
    *seconds = 0;
    *uid = 0;
    *starting_path = argv[argc - 1];
    int opt;
    struct passwd *getname;
    while((opt = getopt(argc, argv, "u:m:")) != -1) {
        switch (opt) { 
            case 'u': 
                *allusers = 0; //change setting to list only inodes owned by user

                if (!(*uid = atoi(optarg))) { //if atoi returns 0 -> optarg = username
                    if (!(getname = getpwnam(optarg))) {
                        fprintf(stderr, "Error while getting the passwd struct for username %s: %s\n", optarg, strerror(errno));
                        exit(1);
                    } else {
                        *uid = getname->pw_uid;
                    }
                }
                break;

            case 'm': 
                *seconds = abs(atoi(optarg));
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

    if ((user = getpwuid(st_uid))) {
        username = user->pw_name;
    } else {
        if (sprintf(username, "%d", st_uid) == -1) {
            fprintf(stderr, "Error converting User ID, %d, to string: %s\n", st_uid, strerror(errno));
            exit(1);
        }
    }

    if ((group = getgrgid(st_gid))) {
        groupname = group->gr_name;
    } else {
        if (sprintf(groupname, "%d", st_gid) == -1) {
            fprintf(stderr, "Error converting Group ID, %d, to string: %s\n", st_gid, strerror(errno));
            exit(1);
        } 
    }

    printf("%s\t%s\t", username, groupname);
}

void printSize(mode_t st_mode, dev_t major, dev_t minor, off_t size) {
    mode_t filetype = st_mode & 0xF000; 
    if (filetype == S_IFBLK || filetype == S_IFCHR) {
        printf("%d, %d ", major, minor);
    } else {
        printf("%lld ", size);
    }
}

void printInfo(struct stat *info, char *pathname) {
    printf("%d\t%d ", (int) info->st_ino, (int) info->st_blksize/1024);
    printMode(info->st_mode); 
    printf("%d  ", info->st_nlink);
    printName(info->st_uid, info->st_gid);
    printSize(info->st_mode, info->st_dev, info->st_rdev, info->st_size);

    char time[100];
    struct tm* currentTime;
    if (!(currentTime = localtime(&(info->st_mtimespec.tv_sec)))) {
        fprintf(stderr, "Error finding the current time: %s\n", strerror(errno));
        exit(1);
    }
    if (!strftime(time, 100, "%b %d %T %Y", currentTime)) {
        fprintf(stderr, "Error formatting the currentTime: %s\n", strerror(errno));
        exit(1);
    }
    printf("%s %s", time, pathname);

    if ((info->st_mode & 0xF000) == S_IFLNK) {
        char buf[256]; // 256 is maximum characters for a pathname
        ssize_t bytesRead;
        if ((bytesRead = readlink(pathname, buf, 256)) == -1) {
            fprintf(stderr, "Error obtaining contents of the symbolic link: %s\n", strerror(errno));
            exit(1);
        }
        buf[bytesRead] = '\0';
        printf(" -> %s", buf);
    }
    printf("\n");
}

int shouldPrint(char* path, struct stat *info, int *uid, int *allusers, int *seconds) {
    time_t currentTime;
    if ((currentTime = time(NULL)) == -1) {
        fprintf(stderr, "Error finding the current time: %s\n", strerror(errno));
        exit(1);
    }
    return (*allusers || info->st_uid == *uid) && currentTime - info->st_mtimespec.tv_sec >= *seconds;
}

//walks from one node to the next
void walk(char *path, struct stat *info, int* uid, int* allusers, int* seconds) {
    if (lstat(path, info) == -1) { 
        fprintf(stderr, "Error finding the information about file %s: %s\n", path, strerror(errno));
        exit(1);
    }

    if (shouldPrint(path, info, uid, allusers, seconds)) { //if the file SHOULD be printed, print info
        printInfo(info, path);
    }

    if((info->st_mode & 0xF000) == S_IFDIR) {
        struct dirent *entry;
        DIR *directory;

        if (!(directory = opendir(path))) { //if directory == NULL
            fprintf(stderr, "Error opening directory stream corresponding to directory %s: %s\n", path, strerror(errno));
            exit(1);
        }

        int length = strlen(path);

        errno = 0;
        while ((entry = readdir(directory)) != NULL) {
            if (*(entry->d_name) == '.' || !strcmp(entry->d_name, "..")) {
                continue;
            }
            char newPath[length + entry->d_namlen];
            strcpy(newPath, path);
            newPath[length] = '/';
            strcpy(newPath + length + 1, entry->d_name);
            walk(newPath, info, uid, allusers, seconds);
        }

        if (errno) {
            fprintf(stderr, "Error reading entries of directory %s: %s\n", path, strerror(errno));
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    struct stat info; 
    char* starting_path;

    int allusers;
    int uid;
    int seconds;

    getArgs(argc, argv, &starting_path, &uid, &allusers, &seconds);
    walk(starting_path, &info, &uid, &allusers, &seconds);

    return 0;
}