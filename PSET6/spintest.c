#include "spinlock.h"

int main(int argc, char **argv) {
    int *memory;
    int numChildren = atoi(argv[1]);
    pid_t pids[numChildren];  
    int numIterations = 100000;
    struct spinlock l = {0,0,0};

    if ((memory = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap: %s\n", strerror(errno));
        return -1;
    }

    for (int i = 0; i < numChildren; i++) {
        if ((pids[i] = fork()) < 0) {
            fprintf(stderr,"Error: fork failed %s\n", strerror(errno));
            exit(1);
        } else if (pids[i] == 0) {
            for (int j = 0; j < numIterations; j++) {
                spin_lock(&l);
                (*memory)++;
                spin_unlock(&l);
            } 
            exit(0); 
        }
    }

    int status;
    pid_t pid;
    int num = numChildren; 
    while (numChildren > 0) {
        pid = wait(&status);
        numChildren--; 
    }

    printf("Expected: %d\nActual: %d\n", numIterations*num, *memory); 
}