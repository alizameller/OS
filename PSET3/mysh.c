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

struct info {
    char *redirFiles[3]; //redirFiles[0] = in, redirFiles[1] = out, redirFiles[2] = err
    int flags[3]; // array to keep track of flags. flags[0] -> stdin, flags[1] -> stdout, flags[2] -> stderr
    int redirs; // redirs is a 3 bit number that represents which streams were redirected
    // Example: 
    // stderrr  stdout  stdin
    //    1       1       1    = 7 -> stdin, stdout, stderr is redirected
    //    0       1       1    = 3 -> stdin, stdout is redirected
    //    0       0       0    = 0 -> none are redirected 
};

struct info *redirIO(char *args, int redirs, struct info *redirInfo) {
    char temp[2000];
    strcpy(temp, args);
    char *filename = strtok(temp, "<>");
    
    if (!strcmp(filename, "2")) {
        filename = strtok(NULL, "<>"); 
    }

    switch(args[0]) {
        case '<':
            if (redirInfo->redirs & 0x1) { //stdin was already redirected
                printf("stdin was already redirected\n");
                break; 
            }
            // Redirect stdin
            redirInfo->redirFiles[0] = strdup(filename);
            redirInfo->flags[0] = O_RDONLY;
            redirInfo->redirs++; 
            break;
        
        case '>':
            if (redirInfo->redirs & 0x2) { //stdout was already redirected
                printf("stdout was already redirected\n");
                break;
            }
            redirInfo->redirFiles[1] = strdup(filename);
            if ((args[1]) == '>') {
                // Append
                redirInfo->flags[1] = O_WRONLY | O_CREAT | O_APPEND; 
            } else {
                // Truncate 
                redirInfo->flags[1] = O_WRONLY | O_CREAT | O_TRUNC; 
            }
            redirInfo->redirs = redirInfo->redirs + 2; 
            break;

        case '2':
            if (redirInfo->redirs & 0x4) { //stderr was already redirected
                printf("stderr was already redirected\n");
                break;
            }
            redirInfo->redirFiles[2] = strdup(filename);
            if ((args[1]) == '>' && ((args[2]) == '>')) {
                // Append
                redirInfo->flags[2] = O_WRONLY | O_CREAT | O_APPEND;
            } else if ((args[1]) == '>' && ((args[2]) >= 1 && ((args[2]) <= 127))) {
                // Truncate
                redirInfo->flags[2] = O_WRONLY | O_CREAT | O_TRUNC;
            }
            redirInfo->redirs = redirInfo->redirs + 4; 
            break;
    }
    return redirInfo; 
}

int shelliza_cd(char **args, int *status) {
    char *path;
    if (args[1] == NULL) {
        char *envvar = "HOME";
        if (!(path = getenv(envvar))){
            fprintf(stderr, "The environment variable %s was not found.\n", envvar);
            return errno; 
        }
    } else {
        path = args[1];
    }

    if (chdir(path) == -1) {
        fprintf(stderr, "Could not change directory to path %s: %s\n", path, strerror(errno));
    }

    return errno; 
}

int shelliza_pwd(char **args, int *status) {
    char *string;
    if (!(string = getcwd(NULL, 0))) {
        fprintf(stderr, "Could not get current working directory: %s\n", strerror(errno));
        return errno; 
    }
    printf("%s\n", string);

    return errno; 
}

int shelliza_exit(char **args, int *status) {
    if (args[1]) {
        *status = atoi(args[1]);
    }
    exit(*status);
}

// Using an array of structs to map the command name to its respective function 
struct functionMap {
    char *name;
    int (*func)(char **args, int *status);
};
// Initialization
struct functionMap functions[] = {
    {"cd", shelliza_cd},
    {"pwd", shelliza_pwd}, 
    {"exit", shelliza_exit}
};

char** tokenization(char *line) {
    int i;
    char *str1;
    char **string = (char **)malloc(2000 * sizeof(char*)); //limit for number of commands

    if(string == NULL) {
        fprintf(stderr, "Error while allocating space for the buffer: %s\n", strerror(errno));
        exit(1); 
    }

    for (i = 0, str1 = line; ; i++, str1 = NULL) {
        string[i] = strtok(str1, " \t"); 
        if (!string[i]) {
            break;
        }
        if (i == 1999) { // last valid index and string[i] is not NULL
            fprintf(stderr, "Error executing command. Too many tokens\n");
            *string[0] = '#'; // overwrite string so that when tokenization returns, the driver function continues to the next command
        }
    }

    return string; 
}

void shelliza_exec(char **args, int *status, struct info *redirInfo) {
    int i;
    int wstatus;
    int j = 0x1; 
    int fd;
    int stdFileno[3] = {STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO};  

    struct rusage rusage;
    struct timeval startTime;
    struct timeval realTime;
    gettimeofday(&startTime, NULL);

    pid_t childPid; 

    if ((childPid = fork()) == 0) { // inside child process
        for (i = 0; i < 3; i++) {
            if (redirInfo->redirs & j) { 
                //printf("%s %s\n", redirInfo->redirFiles[i], redirInfo->flags[i]); 
                if ((fd = open(redirInfo->redirFiles[i], redirInfo->flags[i], 0666)) < 0) {
                    fprintf(stderr, "Error while opening file %s with the flag %d: %s\n", redirInfo->redirFiles[i], redirInfo->flags[i], strerror(errno));
                    exit(1);  
                }
                if (dup2(fd , stdFileno[i]) < 0) {
                    exit(1);
                }
                close(fd);
            }
            j = j << 1; // left shift j to perform the next masking
        }
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, "Error while executing command %s: %s\n", args[0], strerror(errno));
            exit(127);
        }
    } else if (childPid > 0) { // parent process
        if (wait3(&wstatus, 0, &rusage) == -1) {
            fprintf(stderr, "Error waiting for child process %d: %s\n", childPid, strerror(errno));
            *status = errno; 
            return; 
        }

        if (WIFEXITED(wstatus)) { //WIFEXITED -> did proc exit normally (vs. signal termination)
            wstatus >>= 8;
            *status = wstatus; 
            if (wstatus == 0) {
                printf("Child Process %d exited normally\n", childPid); 
            } else {
                printf("Child Process %d exited with return value %d\n", childPid, wstatus);
            }
        } else if (WIFSIGNALED(wstatus)) { //WIFSIGNALED -> did proc exit successfully (exit code = 0)
            wstatus &= 0x00FF; 
            *status = wstatus; 
            printf("Child Process %d exited with signal %d\n", childPid, wstatus);
        }

        gettimeofday(&realTime, NULL); 
        getrusage(RUSAGE_CHILDREN, &rusage); 

        fprintf(stderr, "Real: %06fs ", realTime.tv_sec-startTime.tv_sec + (realTime.tv_usec-startTime.tv_usec)/1e6);
        fprintf(stderr, "User: %06fs ", rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec/1e6);
        fprintf(stderr, "System: %06fs\n", rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec/1e6); 

        return;   
        }
}

void shelliza_builtin(char **args, int *status, struct info *redirInfo) {
    int i; 
    
    for (i = 0; i < 3; i++) { 
        if (!strcmp(args[0], functions[i].name)) { //if the command matches a command in the functions map (struct)
            *status = functions[i].func(args, status); //call the respective built in function and set the exit status
            return;
        }
    }
    shelliza_exec(args, status, redirInfo); // if function is not a built-in function, call exec
}

void driver(FILE *inStream, char *inFileName) {  
    char **execArgs; 
    char *buffer; 
    int status = 0;

    struct info *redirInfo = (struct info *)malloc(sizeof (struct info));

    size_t bufsize = 500; // just a random number 
    buffer = (char *)malloc((bufsize + 1) * sizeof(char));

    if(buffer == NULL) {
        fprintf(stderr, "Error while allocating space for the buffer: %s\n", strerror(errno));
        exit(1);
    }

    while (1) {
        if (inStream == stdin) {
            printf("[shelliza:~]$ ");
        }
        if (getline(&buffer, &bufsize, inStream) == -1) {
            break;
        }
        //printf("%s", buffer);
        buffer[strlen(buffer) -1] = '\0'; 

        char **tokens = tokenization(buffer);
        if ((tokens[0] == NULL) || (*tokens[0] == '#')) {
            free(tokens); 
            continue; 
        }

        // initializing redirection to NULL
        redirInfo->redirFiles[0] = redirInfo->redirFiles[1] = redirInfo->redirFiles[2] = NULL;
        redirInfo->flags[0] = redirInfo->flags[1] = redirInfo->flags[2] = '\0';
        redirInfo->redirs = 0;
        
        // iterate through the strings in tokens
        int i;
        int j; 
        int length;
        int firstRedir = 0;
        for (i = 0; tokens[i]; i++) {
            length = strlen(tokens[i]);
            for (j = 0; j < length; j++) {
                if (tokens[i][j] == '>' || tokens[i][j] == '<') {
                    redirInfo = redirIO(tokens[i], redirInfo->redirs, redirInfo); 
                    j = length; 
                    if (!firstRedir) {
                        firstRedir = i;
                    }
                }
            }
        }

        if (firstRedir) {
            tokens[firstRedir] = NULL;
        }
        shelliza_builtin(tokens, &status, redirInfo);

        free(tokens); 
    }

    if (errno) {
        fprintf(stderr, "Error while reading line from %s: %s\n", inFileName, strerror(errno));
        exit(1);
    }

    printf("end of file read, exiting shell with exit code %d\n", status);

    free(buffer);
    free(redirInfo); 

    exit(status);
}

int main(int argc, char **argv) {
    FILE *inStream;
    char *inFileName;
    char *pathname;
    int fd;

    // if shell script
    if ((pathname = argv[1]) != NULL) {
        inFileName = pathname;
        inStream = fopen(pathname, "r");
    } else { // else if not shell script 
        inFileName = "STDIN"; 
        inStream = stdin; 
    }

    driver(inStream, inFileName); 

    return 0;
}