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

void redirIO(char **args) {
    int i;
    int j;
    // finding index at which redirection occurs
    for (i = 0; args[i]; i++) {
        for (j = 0; j < strlen(args[i]); j++) {
            if ((args[i])[j] == '<' || (args[i])[j] == '>') {
                printf("%s\n", args[i]); //error checking
            }
        }
    }
}

int shelliza_cd(char **args) {
    char *path;
    if (args[1] == NULL) {
        char *envvar = "HOME";
        if (!(path = getenv(envvar))){
            fprintf(stderr, "The environment variable %s was not found.\n", envvar);
            return 2; 
        }
    } else {
        path = args[1];
    }

    if (chdir(path) == -1) {
        fprintf(stderr, "Could not change directory to path %s: %s\n", path, strerror(errno));
        return 1; 
    }

    return 0; 
}

int shelliza_pwd(char **args) {
    char *string;
    if (!(string = getcwd(NULL, 0))) {
        fprintf(stderr, "Could not get current working directory: %s\n", strerror(errno));
        return 1;
    }
    printf("%s\n", string);
    free(string);

    return 0; 
}

int shelliza_exit(char **args) {
    int status;
    if (args[1] == NULL || args[0] == NULL) {
        status = 0; //change this to exit status of last command
    } else {
        status = atoi(args[1]);
    }
    exit(status);
}

// Using an array of structs to map the command name to its respective function 
struct functionMap {
    char *name;
    int (*func)(char **args);
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
    }

    return string; 
}

void shelliza_exec(char **args) {
    int i;
    int wstatus; 
    struct rusage rusage;
    clock_t realTime; 
    realTime = clock();
    pid_t childPid = fork();

    if (childPid == 0) { // inside child process
        execvp(args[0], args);
        fprintf(stderr, "Error while executing command %s: %s\n", args[0], strerror(errno));
        exit(1);
    } else if (childPid > 0) { // parent process
        do {
            if (wait3(&wstatus, 0, &rusage) == -1) {
                //error 
                exit(wstatus);
            }

            if (WIFEXITED(wstatus)) { //WIFEXITED -> did proc exit normally (vs. signal termination)
                wstatus >> 8;
                if (wstatus == 0) {
                    printf("Child Process %d exited normally\n", childPid); 
                } else {
                    printf("Child Process %d exited with return value %d\n", childPid, wstatus);
                }
            } else if (WIFSIGNALED(wstatus)) { //WIFSIGNALED -> did proc exit successfully (exit code = 0)
                wstatus & 0x00FF; 
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

void shelliza_builtin(char **args) {
    int i; 
    int builtin = 0;
    for (i = 0; i < 3; i++) {
        if (!strcmp(args[0], functions[i].name)) { //if the command matches a command in the functions map (struct)
            builtin = 1; 
            functions[i].func(args); //call the respective function
            return;
        }
    }
    shelliza_exec(args); // if function is not a built-in function, call exec
}

void driver(FILE *stream, char *inFileName, char *outFileName) {  
    char **execArgs; 
    char *buffer; 
    size_t bufsize = 500; // just a random number 
    buffer = (char *)malloc((bufsize + 1) * sizeof(char));

    if(buffer == NULL) {
        fprintf(stderr, "Error while allocating space for the buffer: %s\n", strerror(errno));
        exit(1);
    }

    while (1) {
        if (stream == stdin) {
            printf("[shelliza:~]$ ");
        }
        if (getline(&buffer, &bufsize, stream) == -1) {
            break;
        }
        char **tokens = tokenization(buffer);
        // tokens[0, 1] = command + arguments ?
        // tokens[2, 3] = redirection operations ?
        if (*tokens[0] == '#') {
            continue; 
        }

        redirIO(tokens); 

        if (tokens[0] != NULL) {
            shelliza_builtin(tokens);
        }

        free(tokens); 
    }

    if (errno) {
        fprintf(stderr, "Error while reading line from %s: %s\n", inFileName, strerror(errno));
        exit(1);
    }

    free(buffer); 
    return;
}

int main(int argc, char **argv) {
    FILE *stream;
    char *inFileName;
    char *outFileName;
    char *pathname;
    int fd;

    // if shell script
    if ((pathname = argv[1]) != NULL) {
        inFileName = pathname;
        stream = fopen(pathname, "r");
    } else { // else if not shell script and no IO redirection
        inFileName = "STDIN"; 
        outFileName = "STDOUT"; 
        stream = stdin; 
    }

    driver(stream, inFileName, outFileName); 

    return 0;
}
