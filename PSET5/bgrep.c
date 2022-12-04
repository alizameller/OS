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
#include <ctype.h> 

/*jmp_buf jumpBuf;

void sigHandler(int signum) {
    if (signum == SIGBUS) {
        return;
    }
} */

int compare(char *pattern, char *file, int context) {
    //printf ("Input File: %s\n", file);
    int size;
    struct stat st; 
    int file_fd;
    char *info;
    int i; 
    int ret;

    if ((file_fd = open(file, O_RDONLY)) == -1) {
        fprintf(stderr,"Error: could not open %s for reading: %s\n", file, strerror(errno));
        return -1; 
    }

    // Get size of file
    stat(file, &st);
    size = st.st_size;

    if ((info = mmap(NULL, size, PROT_READ,MAP_SHARED, file_fd, 0)) == MAP_FAILED) {
        close(file_fd); 
        fprintf(stderr,"Error: could not mmap %s: %s\n", file, strerror(errno));
        return -1;
    }

    close(file_fd); 

    int pattern_length = strlen(pattern);
    int matches = 0;
 
    char *start = info;
    char *end = info + size; 
    char *info_start;
    char *info_end; 
    int j = 0;
    
    for (i = 0; i < size - pattern_length + 1; i++) {
        ret = memcmp(pattern, info + i, pattern_length);
        if (!ret) {
            matches++; 
            printf("%s:%d ", file, i); 
            info_start = info + i;
            info_end = info_start + pattern_length; 

            if (context) {
                //info_end  = info_end + context; 
                if ((info_end += context) > end) {
                    info_end = end; 
                }
                if ((info_start -= context) < start) {
                    info_start = start; 
                }
                
                // printing readable characters
                for(j = 0; j < info_end - info_start; j++) {
                    if (isprint(info_start[j])) {
                        printf("%C ", info_start[j]); 
                    } else {
                        printf("? ");
                    }
                }

                printf(" "); 

                // printing in hex characters
                for(j = 0; j < info_end - info_start; j++) {
                    printf("%X ", info_start[j]); 
                }

                printf("\n"); 
            }
        }
    }

    return matches;
}

int driver(int argc, char** argv) {
    int i;
    int opt;
    int c = 0;
    int p = 0;

    int context = 0; 
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
                context = atoi(optarg); 
                //printf("context bytes: %d\n", context);

                break;
            case 'p': 

                // Read the pattern from pattern_file
                pattern_file = optarg;
                if ((pattern_fd = open(pattern_file, O_RDONLY)) == -1) {
                    fprintf(stderr,"Error: could not open %s for reading: %s\n", optarg, strerror(errno));
                    return -1; 
                }

                // Get size of file
                stat(pattern_file, &st);
                size = st.st_size;
                
                // Call mmap to read pattern file and store pattern in p (an array of chars)
                if ((pattern = mmap(NULL, size, PROT_READ,MAP_SHARED, pattern_fd, 0)) == MAP_FAILED) {
                    close(pattern_fd); 
                    fprintf(stderr,"Error: could not mmap %s: %s\n", optarg, strerror(errno));
                    return -1;
                }

                close(pattern_fd); 
                
                p = 1; 
        }
    }

    if (!p) { // if -p was not used, the pattern is the first argument after the options
        pattern = argv[optind++];
    }

    int val; 
    int error_occured = 0; 

    if (optind >= argc) { // no input files provided
        fprintf(stderr,"Error: can not use STDIN as input to bgrep: %s\n", strerror(errno));
        exit(1); 
    } else {
        for (index = optind; index < argc; index++) { 
            val = compare(pattern, argv[index], context);
            if (val == -1) {
                error_occured = 1; 
            } 
            //printf("found %d matches\n", val); 
        }
    }

    if (error_occured) {
        return -1; 
    } 

    return !val;   
}

int main(int argc, char **argv) { 
    //errno = 0;
    //int i; 

    /*struct sigaction sa;
    sa.sa_handler = sigHandler;
    sa.sa_flags = SA_RESTART;   

    if (sigaction(SIGUSR1, &sa, NULL) == -1 || sigaction(SIGUSR2, &sa, NULL) == -1) {  
        fprintf(stderr, "Error while setting signal: %s\n", strerror(errno));
        exit(1);
    } */
    return driver(argc, argv); 
}