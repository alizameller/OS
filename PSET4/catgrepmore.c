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

int pipe1[2];
int pipe2[2]; 

int main(int argc, char **argv) {
    int fd; 
    int i;

    char buf[4096];
    int bytesRead;
    int bytesWritten;

    pid_t grepPid; 
    pid_t morePid; 
    int wstatus;

    for (i = 2; i < argc; i++) {
        if ((fd = open(argv[i], O_RDONLY, 0666)) < 0) {
            fprintf(stderr, "Error while opening file %s: %s\n", argv[i], strerror(errno));
            exit(1);  
        }
        
        pipe(pipe1);
        pipe(pipe2);

        if (!(grepPid = fork())) { //grep with sole argument = pattern
            close(pipe1[1]);
            close(pipe2[0]);
            //stdin = pipe1[0] // read side of pipe1
            //stdout = pipe2[1] // write side of pipe2
            if (dup2(pipe1[0] , STDIN_FILENO < 0)) {
                exit(1);
            }
            close(pipe1[0]);

            if (dup2(pipe2[1] , STDOUT_FILENO < 0)) {
                exit(1);
            }
            close(pipe2[1]);

            char *grepArgs[2] = {argv[1], NULL}; 
            
            if (execvp("grep", grepArgs) < 0) {
                fprintf(stderr, "Error while executing grep with pattern %s: %s\n", argv[1], strerror(errno));
                exit(1); //exit?
            }
        }
        
        if (!(morePid = fork())) { //more 
            close(pipe2[1]); 
            //stdin = pipe2[0] // read side of pipe2
            //stdout = stdout
            if (dup2(pipe2[0] , STDIN_FILENO < 0)) {
                exit(1);
            }
            close(pipe1[0]);

            if (execvp("more", NULL) < 0) {
                fprintf(stderr, "Error while executing more: %s\n", strerror(errno));
                exit(1); //exit?
            }
        } else {
            close(pipe1[0]); // close read side of pipe1 because we are about to write to the write side of pipe1
            close(pipe2[0]);
            close(pipe2[1]);
            
            while ((bytesRead = read(fd, buf, sizeof buf)) > 0) { //read from file and write to pipe1
                if ((bytesWritten = write(pipe1[1], buf, bytesRead)) < bytesRead) {
                    if (bytesWritten == -1) {
                        //error but NOT FATAL
                    }
                    // partial write?
                }
            } // reading EOF returns 0, but errno = 0
            if (errno != 0) {
                //error but NOT FATAL
                if (errno == EINTR) {
                    //make some decision
                }
            }
            close(pipe1[1]);
            close(fd);

            pid_t pids[2] = {grepPid, morePid};
            //before moving on to next file, call wait twice to avoid leaving zombie processes (the grep and the more processes).
            for (i = 0; i < 2; i++) {
                if (waitpid(pids[i], &wstatus, 0) == -1) {
                    fprintf(stderr, "Error waiting for child process %d: %s\n", pids[i], strerror(errno));
                    return 0;
                }
            }
        }
    }
    return 0; 
}