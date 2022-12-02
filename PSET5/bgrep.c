#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>

/*jmp_buf jumpBuf;

void sigHandler(int signum) {
    if (signum == SIGBUS) {
        return;
    }
} */

int compare(char *pattern, char *file, int c) {
    printf ("Input File: %s\n", file);
    return 0;
}

void getArgs(int argc, char** argv) {
    int opt;
    int c = 0;
    int p = 0;

    char *pattern_file;
    int size;
    struct stat st;
    int pattern_fd; 
    char* pattern; 

    int index;
    int offset; 

    while((opt = getopt(argc, argv, "c:p:")) != -1) {
        switch (opt) { 
            case 'c': 
                // output the binary context
                c = 1; 
                printf("context context context!\n"); 

                break;
            case 'p': 

                // Read the pattern from pattern_file
                pattern_file = optarg;
                if ((pattern_fd = open(pattern_file, O_RDONLY)) == -1) {
                    fprintf(stderr,"Error: could not open %s for reading: %s\n", optarg, strerror(errno));
                    exit(1); 
                }

                // Get size of file
                stat(pattern_file, &st);
                size = st.st_size;
                
                // Call mmap to read pattern file and store pattern in p (an array of chars)
                if ((pattern = mmap(NULL, size, PROT_READ,MAP_SHARED, pattern_fd, 0)) == MAP_FAILED) {
                    fprintf(stderr,"Error: could not mmap %s: %s\n", optarg, strerror(errno));
                    exit(1);
                }
                
                p = 1; 

                break;
        }
    }
    // just a check
    printf("%s\n", pattern); 

    if (!p) { // if -p was not used, the pattern is the first argument after the options
        printf("p = 0\n");
        pattern = argv[optind];
        optind++; 
    }

    if (optind >= argc) { // i.e. no input files provided
        printf("input is stdin\n");
    } else {
        for (index = optind; index < argc; index++) { 
            offset = compare(pattern, argv[index], c); 
            printf("%d\n", offset); 
        }
    }

    return; 
}

int main(int argc, char **argv) {
    getArgs(argc, argv); 
    //errno = 0;
    //int i; 

    /*struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_flags = SA_RESTART;   

    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGUSR2, &sa, NULL) == -1) {  
        fprintf(stderr, "Error while setting signal: %s\n", strerror(errno));
        exit(1);
    } */
    return 0; 
}