#include "fifo.h"

// Note: checking for correctness takes a long time for a large number of writers
//       please allow to run for several minutes

void test1(struct fifo* f) {
    pid_t pids[2];
    int numWrites = 65536;
    int* testArr = (int*)calloc(numWrites, sizeof(int));
    long temp;

    for (int i = 0; i < 2; i++) {
        if ((pids[i] = fork()) < 0) {
            fprintf(stderr,"Error: fork failed %s\n", strerror(errno));
            exit(1);
        } else if (pids[i] == 0) {
            if (!i) { //writer
                for (int j = 0; j < numWrites; j++) {
                    fifo_wr(f, j);
                }
            } else { //reader
                while (1) {
                    temp = fifo_rd(f);
                    testArr[temp]++;
                    if (temp && !testArr[temp - 1]) {
                        printf("Test 1 Failed\n");
                        exit(0);
                    }
                    if (temp == numWrites - 1) {
                        break;
                    }
                }
                for (int i = 0; i < numWrites; i++) {
                    if (testArr[i] != 1) {
                        printf("Test 1 Failed\n");
                        exit(0);
                    }
                }
                printf("Test 1 Passed!\n");
            }
            exit(0);
        }
    }

    int status;

    for(int i = 0; i < 2; i++) {
        wait(&status); 
    }

    free(testArr);
}

void test2 (struct fifo* f) {
    int numChildren = 64;
    int numWrites = 65536;
    int* testMat[numChildren];
    for (int i = 0; i < numChildren; i++) {
        testMat[i] = (int*)calloc(numWrites, sizeof(int));
    }

    pid_t pids[numChildren];

    for (int i = 0; i < numChildren; i++) {
        if ((pids[i] = fork()) < 0) {
            fprintf(stderr,"Error: fork failed %s\n", strerror(errno));
            exit(1);
        } else if (pids[i] == 0) {
            for (int j = 0; j < numWrites; j++) {
                fifo_wr(f, (i << 16) | j);
            } 
            exit(0);
        }
    }

    int sum = 0;
    long temp;
    int writerID;
    int num;
    while (1) {
        temp = fifo_rd(f);
        writerID = temp >> 16;
        num = temp & 0xFFFF;

        testMat[writerID][num]++;
        if (num && !testMat[writerID][num-1]) {
            printf("Test 2 Failed\n");
            exit(0);
        }

        if (num == 0xFFFF) {
            if (++sum == numChildren) {
                break;
            }
        }
    }

    for (int i = 0; i < numChildren; i++) {
        for (int j = 0; j < numWrites; j++) {
            if (testMat[i][j] != 1) {
                printf("Test 2 Failed\n");
                exit(0);
            }
        }
    }
    printf("Test 2 Passed!\n");

    int status;

    for(int i = 0; i < numChildren; i++) {
        wait(&status); 
    }

    for (int i = 0; i < numChildren; i++) {
        free(testMat[i]);
    }
}

int main() {
    struct fifo* f;
    if ((f = (struct fifo*) mmap(NULL, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap struct fifo: %s\n", strerror(errno));
        return -1;
    }

    fifo_init(f);
    test1(f);


    fifo_init(f);
    test2(f);
}
