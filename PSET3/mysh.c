#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

struct info {
    char *redirFiles[3]; //redirFiles[0] = in, redirFiles[1] = out, redirFiles[2] = err
    char *flags[3]; // array to keep track of flags. flags[0] -> stdin, flags[1] -> stdout, flags[2] -> stderr
    int redirs; // redirs is a 3 bit number that represents which streams were redirected
    // Example: 
    // stderrr  stdout  stdin
    //    1       1       1    = 7 -> stdin, stdout, stderr is redirected
    //    0       1       1    = 3 -> stdin, stdout is redirected
    //    0       0       0    = 0 -> none are redirected 
};

struct info *redirIO(char *args, int redirs, struct info *redirInfo) {
    char *temp = args;
    char *filename = strtok(temp, "<>2");
    //printf("%s\n", filename); 
    switch(args[0]) {
        case '<':
            if (redirInfo->redirs & 0x1) { //stdin was already redirected
                printf("stdin was already redirected\n");
                break; 
            }
            // Redirect stdin
            redirInfo->redirFiles[0] = strdup(filename);
            redirInfo->flags[0] = strdup("r");
            //printf("%s\n", redirInfo->redirIn); 
            redirInfo->redirs++; 
            //error
            break;
        
        case '>':
            if (redirInfo->redirs & 0x2) { //stdout was already redirected
                printf("stdout was already redirected\n");
                break;
            }
            redirInfo->redirFiles[1] = strdup(filename);
            if ((args[1]) == '>') {
                // Append
                redirInfo->flags[1] = strdup("a"); 
            } else {
                // Truncate 
                redirInfo->flags[1] = strdup("w"); 
            }
            redirInfo->redirs = redirInfo->redirs + 2; 
            //printf("%d\n", redirs); 
            //error
            break;

        case '2':
            if (redirInfo->redirs & 0x4) { //stderr was already redirected
                printf("stderr was already redirected\n");
                break;
            }
            redirInfo->redirFiles[2] = strdup(filename);
            if ((args[1]) == '>' && ((args[2]) == '>')) {
                // Append
                redirInfo->flags[2] = strdup("a"); 
            } else if ((args[1]) == '>' && ((args[2]) >= 1 && ((args[2]) <= 127))) {
                // Truncate
                redirInfo->flags[2] = strdup("w"); 
            }
            redirInfo->redirs = redirInfo->redirs + 4; 
            //printf("%d\n", redirs); 
            //error
            break;
    }
    //printf("%d\n", redirs); 
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
        string[i] = strtok(str1, " \t\n"); 
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

    struct rusage rusage;
    clock_t realTime; 
    realTime = clock();
    pid_t childPid = fork();

    FILE *streams[3] = {stdin, stdout, stderr}; 

    int j = 0x1; 

    if (childPid == 0) { // inside child process
        for (i = 0; i < 3; i++) {
            if (redirInfo->redirs & j) { 
                printf("%s %s\n", redirInfo->redirFiles[i], redirInfo->flags[i]); 
                if ((streams[i] = fopen(redirInfo->redirFiles[i], redirInfo->flags[i]))) {
                    fprintf(stderr, "Error while opening file %s for reading: %s\n", redirInfo->redirFiles[i], strerror(errno));
                    exit(1);  
                }
                if (dup2(fileno(streams[i]), i)) {
                    fprintf(stderr, "Error while obtaining file descriptor for file %s: %s\n", redirInfo->redirFiles[i], strerror(errno));
                    exit(1);
                }
                close(fileno(streams[i])); // closing the file descriptor to keep a clean file descriptor environment
            }
            j = j << 1; // left shift j to perform the next masking
        }

        execvp(args[0], args);

        fprintf(stderr, "Error while executing command %s: %s\n", args[0], strerror(errno));
        exit(1);
    } else if (childPid > 0) { // parent process
        do {
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
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus)); 
            realTime = clock() - realTime; 
            getrusage(RUSAGE_CHILDREN, &rusage); 

            printf("Real: %fs ", ((double)realTime)/CLOCKS_PER_SEC);
            printf("User: %ld.%06ds ", rusage.ru_utime.tv_sec, rusage.ru_utime.tv_usec);
            printf("System: %ld.%06ds\n", rusage.ru_stime.tv_sec, rusage.ru_stime.tv_usec); 
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

    struct info *redirInfo = malloc(sizeof *redirInfo);
    // initializing redirection to NULL
    redirInfo->redirFiles[0] = NULL;
    redirInfo->redirFiles[1] = NULL;
    redirInfo->redirFiles[2] = NULL;

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

        char **tokens = tokenization(buffer);
        if ((tokens[0] == NULL) || (*tokens[0] == '#')) {
            free(tokens); 
            continue; 
        }

        // iterate through the strings in tokens
        // count number of '<' + number of '>' + number of '2>' + number of '>>' + number of '2>>'
        int i;
        int j; 
        int length;
        for (i = 0; tokens[i]; i++) {
            length = strlen(tokens[i]);
            for (j = 0; j < length; j++) {
                if (tokens[i][j] == '>' || tokens[i][j] == '<') {
                    redirInfo = redirIO(tokens[i], redirInfo->redirs, redirInfo); 
                    j = length; 
                }
            }
        }

        shelliza_builtin(tokens, &status, redirInfo);

        free(tokens); 
    }

    if (errno) {
        fprintf(stderr, "Error while reading line from %s: %s\n", inFileName, strerror(errno));
        exit(1);
    }

    free(buffer); 
    //free(redirInfo);
    return;
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