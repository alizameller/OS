#include "spinlock.h"

int main(int argc, char **argv) {
    int numChildren = atoi(argv[1]);
    pid_t pids[numChildren];

    struct spinlock *l;
    if ((l = (struct spinlock*) mmap(NULL, sizeof(struct spinlock), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap struct spinlock: %s\n", strerror(errno));
        return -1;
    }
    spin_init(l);

    int *spinlock_memory;
    if ((spinlock_memory = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap memory region: %s\n", strerror(errno));
        return -1;
    }

    int *nolock_memory;
    if ((nolock_memory = (int*) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap memory region: %s\n", strerror(errno));
        return -1;
    }

    int numIterations = 100000;
    int i;
    int j;
    for (i = 0; i < numChildren; i++) {
        if ((pids[i] = fork()) < 0) {
            fprintf(stderr,"Error: fork failed %s\n", strerror(errno));
            exit(1);
        } else if (pids[i] == 0) {
            for (j = 0; j < numIterations; j++) {
                // using spinlock
                spin_lock(l);
                (*spinlock_memory)++;
                spin_unlock(l);

                // not using spinlock 
                (*nolock_memory)++; 
            } 
            exit(0); 
        }
    }

    int status;

    for(i = 0; i < numChildren; i++) {
        wait(&status); 
    }

    printf("Expected:            %d\nResult (spinlocked): %d\nResult (unlocked):   %d\n", numIterations*numChildren, *spinlock_memory, *nolock_memory); 
}