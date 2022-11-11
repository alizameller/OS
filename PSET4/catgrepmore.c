#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <setjmp.h>

int pipe1[2];
int pipe2[2]; 
jmp_buf jumpBuf;
int totalFiles;
int numFiles;
int bytesProcessed;

void sigHandler(int signum) {
    if (signum == SIGUSR1) {
        fprintf(stderr, "*** SIGUSR1 received *** Files Processed: %d Bytes Processed: %d\n", numFiles, bytesProcessed);
    } else if (signum == SIGUSR2) {
        if (numFiles == totalFiles) {
            fprintf(stderr, "*** SIGUSR2 received *** no more files. Exiting catgrepmore\n");
        } else {
            fprintf(stderr, "*** SIGUSR2 received *** moving on to file #%d\n", numFiles + 1);
        }
        longjmp(jumpBuf, 1); // Jump to the point located by setjmp
    }
}

int main(int argc, char **argv) {
    totalFiles = argc - 2; 
    int fd; 
    int i;
    int j;

    char buf[4096];
    int bytesRead;
    int bytesWritten;

    pid_t grepPid; 
    pid_t morePid; 
    int wstatus;

    struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_flags = SA_RESTART;   

    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGUSR2, &sa, NULL) == -1) {  
        fprintf(stderr, "Error while setting signal: %s\n", strerror(errno));
        exit(1);
    } 

    for (i = 2; i < argc; i++) {
        numFiles++; 

        if ((fd = open(argv[i], O_RDONLY, 0666)) < 0) {
            fprintf(stderr, "Error while opening file %s: %s\n", argv[i], strerror(errno));
            exit(1);  
        }
        
        if (pipe(pipe1) || pipe(pipe2)) {
            fprintf(stderr, "Bad pipe: %s\n", strerror(errno));
            exit(1);
        }

        if (!(grepPid = fork())) { //grep with sole argument = pattern
            close(pipe1[1]);
            close(pipe2[0]);
            //stdin = pipe1[0] // read side of pipe1
            //stdout = pipe2[1] // write side of pipe2
            if (dup2(pipe1[0] , STDIN_FILENO < 0)) {
                fprintf(stderr, "Error duping read side of pipe1 to stdin of grep: %s\n", strerror(errno));
                exit(1);
            }
            close(pipe1[0]);

            if (dup2(pipe2[1] , STDOUT_FILENO) < 0) {
                fprintf(stderr, "Error duping write side of pipe2 to stdout of grep: %s\n", strerror(errno));
                exit(1);
            } 
            close(pipe2[1]);

            char *grepArgs[3] = {"grep", argv[1], (char *) NULL}; 
            execvp("grep", grepArgs);
            fprintf(stderr, "Error while executing grep with pattern %s: %s\n", argv[1], strerror(errno));
            exit(1); //exit?

        } else if (!(morePid = fork())) { //more 
            close(pipe1[0]);
            close(pipe1[1]); 
            close(pipe2[1]); 
            //stdin = pipe2[0] // read side of pipe2
            //stdout = stdout
            if (dup2(pipe2[0] , STDIN_FILENO) < 0) {
                fprintf(stderr, "Error duping read side of pipe2 to stdin of more: %s\n", strerror(errno));
                exit(1);
            }
            close(pipe2[0]);

            char *moreArgs[2] = {"more", (char *) NULL}; 
            execvp("more", moreArgs);
            fprintf(stderr, "Error while executing more: %s\n", strerror(errno));
            exit(1); //exit?

        } else {
            close(pipe1[0]); // close read side of pipe1 because we are about to write to the write side of pipe1
            close(pipe2[0]);
            close(pipe2[1]);

            while ((bytesRead = read(fd, buf, sizeof buf)) > 0) { //read from file and write to pipe1
                bytesProcessed = bytesProcessed + bytesRead;
                if ((bytesWritten = write(pipe1[1], buf, bytesRead)) < bytesRead) {
                    // partial write occured
                    fprintf(stderr, "Partial write occurred\n");
                }
            } 

            if (errno != 0) {
                fprintf(stderr, "Some error occured during Read or Write system calls: %s\n", strerror(errno));
            }

            if (setjmp(jumpBuf) != 0) {
                close(fd);
                close(pipe1[1]);
                close(pipe1[0]);
                close(pipe2[1]);
                close(pipe2[0]);
                continue;
            } 

            close(fd);
            close(pipe1[1]);

            pid_t pids[2] = {grepPid, morePid};

            //before moving on to next file, call wait twice to avoid leaving zombie processes (the grep and the more processes).
            for (j = 0; j < 2; j++) {
                if (waitpid(pids[j], &wstatus, 0) == -1) {
                    fprintf(stderr, "Error waiting for child process %d: %s\n", pids[j], strerror(errno));
                    exit(1);
                }
            }
        }
    }
    return 0; 
}