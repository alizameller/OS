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

}

void shelliza_cd(char **args) {
    char *path;
    if (args[1] == NULL) {
        char *envvar = "HOME";
        if (!(path = getenv(envvar))){
            fprintf(stderr, "The environment variable %s was not found.\n", envvar);
            exit(1);
        }
    } else {
        path = args[1];
    }
    if (chdir(path) == -1) {
        fprintf(stderr, "Could not change directory to path %s: %s\n", path, strerror(errno));
        exit(1);
    }
}

void shelliza_pwd(char **args) {
    char *string;
    if (!(string = getcwd(NULL, 0))) {
        fprintf(stderr, "Could not get current working directory: %s\n", strerror(errno));
        exit(1);
    }
    printf("%s\n", string);
    free(string);
}

void shelliza_exit(char **args) {
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
    void (*func)(char **args);
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
/*  int wstatus; 
    //split args into commands and IO redirection
    pid_t childPid = fork();

    if (childPid == 0) { // inside child process
        // do IO redirection ?
        redirIO(args); 
        // execute commands
    } else if (childPid > 0) { // parent process
        do {
            if (wait(&wstatus) == -1) {
                //error
                exit(1);
            }

            if (WIFEXITED(wstatus)) {
                printf("exited, status=%d\n", wstatus >> 8);
            } else if (WIFSIGNALED(wstatus)) {
                printf("killed by signal %d\n", wstatus & 0x00FF);
            }

        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
            exit(0);    
    } */
}

void shelliza_builtin(char **args) {
    int i; 
    int numFuncs = 3; 
    for (i = 0; i < numFuncs; i++) {
        if (!strcmp(args[0], functions[i].name)) { //if the command matches a command in the functions map (struct)
            functions[i].func(args); //call the respective function
        }
    }
}

void driver(FILE *stream) {  
    int i;
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
        /*
        for (i = 0; tokens[i]; i++) {
            printf("%s\n", tokens[i]);
        }
        */
        if (tokens[0] != NULL) {
            shelliza_builtin(tokens);
            //shelliza_exec(tokens);
        }

        free(tokens); 
    }

    if (errno) {
        fprintf(stderr, "Error while reading line from %s: %s\n", "INSERT FILENAME", strerror(errno));
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
        stream = fopen(pathname, "r");
    } else { // else if not shell script
        //inFileName = "STDIN"; 
        //outFileName = "STDOUT"; 
        stream = stdin; 
    }

    driver(stream); 

    return 0;
}