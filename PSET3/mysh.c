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

void redirIO(char **args) {

}

void shelliza_cd(char **args) {

}

void shelliza_pwd(char **args) {

}

void shelliza_exit(char **args) {
    
}

struct functionMap {
    char *name;
    void (*func)(char **args);
};

struct functionMap functions[] = {
    {"cd", shelliza_cd},
    {"pwd", shelliza_pwd}, 
    {"exit", shelliza_exit}
};

char* readLine(FILE *stream) {
    char *buffer; 
    size_t bufsize = 0; // how big should I make this
    buffer = (char *)malloc(bufsize * sizeof(char));

    if(buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

    getline(&buffer, &bufsize, stdin);
    // if first character in line is #, ignore the line

    return buffer; 
}

char** tokenization(char *line) {
    int i = 0;
    char **string = (char **)malloc(strlen(line) * sizeof(char));

    char *token = strtok(line, " "); 
    while(token != NULL) {
        string[i++] = token;
        token = strtok(NULL, " ");
    }

    string[strlen(line)] = NULL;
    /*for (i = 0; i < strlen(line); ++i) 
        printf("%s\n", string[i]); */

    return string; 
}

void shelliza_exec(char **args) {
    int i;
    int numFuncs = 3; 
    int wstatus; 
    //split args into commands and IO redirection
    pid_t childPid = fork();

    if (childPid == 0) { //inside child process
        // do IO redirection
        redirIO(args); 
        // execute commands
        for (i = 0; i < numFuncs; i++)
            // execute respective commands inside child process
            if (!strcmp(args[0], functions[i].name)) { //if the command matches a command in the functions map (struct)
                functions[i].func(args); //call the respective functions
            }
        exit(1); 
    } else if (childPid > 0) {
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
    }
}

void driver() {
    printf("[shelliza:~]$ ");
    // if not a shell script
    char *line = readLine(stdin);
    char **tokens = tokenization(line);
    // tokens[0, 1] = command + arguments ?
    // tokens[2, 3] = redirection operations ?
    if (tokens[0] != NULL) {
            shelliza_exec(tokens);
        }
    free(line); 
    free(tokens); 
    return;
}

int main() {
    driver(); 

    return 0;
}