#include "library.c"
#include <time.h>

// Helper function so that all four cases are accounted for
void set_streams(int argc, char** argv, struct MYSTREAM** writeStream, struct MYSTREAM** readStream){
    int opt;
    int write = 0;
    int read = 0;
    char* pathname;
    int bufsiz = 4096;

    while((opt = getopt(argc, argv, "s:o:")) != -1) {
        switch (opt) { 
            case 's': 
                //printf("opening for writing: %s\n", optarg); 
                bufsiz = atoi(optarg);

                if (bufsiz <= 0) {
                    fprintf(stderr, "Error: buffer size must be greater than 0\n");
                    exit(1); 
                }

                break;

            case 'o': 
                pathname = optarg; 
            
                write = 1; // a write stream is going to be opened with given pathname  
        }
    }

    if (!write){ // a write stream is not yet open 
        *writeStream = myfdopen(STDOUT_FILENO, O_WRONLY, bufsiz);

        if (*writeStream == NULL){
            fprintf(stderr,"Error: could not open STDOUT for writing: %s\n", strerror(errno));
            exit(1); 
        }
    } else { // opening write stream with pathname
        *writeStream = myfopen(pathname, O_WRONLY, bufsiz);
            
        if (*writeStream == NULL){
            fprintf(stderr,"Error: could not open %s for writing: %s\n", pathname, strerror(errno));
            exit(1); 
        }
    }

    for(; optind < argc; optind++){ 
        //printf("opening for reading: %s\n", argv[optind]);
        *readStream = myfopen(argv[optind], O_RDONLY, bufsiz);
        
        if (*readStream == NULL){
            fprintf(stderr,"Error: could not open %s for reading: %s\n", argv[optind], strerror(errno));
            exit(1); 
        }
        read = 1; // a read stream is open
    }

    if (!read){ // a read stream is not yet open
        *readStream = myfdopen(STDIN_FILENO, O_RDONLY, bufsiz);

        if (*readStream == NULL){
            fprintf(stderr,"Error: could not open STDOUT for reading: %s\n", strerror(errno));
            exit(1); 
        }
    }
}

int main(int argc, char** argv){
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    struct MYSTREAM* writeStream;
    struct MYSTREAM* readStream;

    set_streams(argc, argv, &writeStream, &readStream);

    int val = 0;
    int check = 0;
    int i;

    while((val = myfgetc(readStream)) != -1) { 
        if (val == '\t'){
            for (i = 0; i < 4; i++){
                check = myfputc(' ', writeStream);

                if (check == -1) {
                    if (writeStream->fd == STDOUT_FILENO){
                        fprintf(stderr,"Error: could not put char to STDOUT: %s\n", strerror(errno));
                        exit(1); 
                    } 
                    fprintf(stderr,"Error: could not put char to file: %s\n", strerror(errno));
                    exit(1); 
                }
            }
        
            continue;
        }

        check = myfputc(val, writeStream);

        if (check == -1){
            if (writeStream->fd == STDOUT_FILENO){
                fprintf(stderr,"Error: could not put char to STDOUT: %s\n", strerror(errno));
                exit(1); 
            } 
            fprintf(stderr,"Error: could not put char to file: %s\n", strerror(errno));
            exit(1); 
        }
    }

    if (errno) {
        if (readStream->fd == STDIN_FILENO){
            fprintf(stderr,"Error: could not get char from STDIN: %s\n", strerror(errno));
            exit(1); 
        } 
        fprintf(stderr,"Error: could not get char from file: %s\n", strerror(errno));
        exit(1); 
    }

    check = myfclose(writeStream);

    if (check == -1){
        if (writeStream->fd == STDOUT_FILENO){
            fprintf(stderr,"Error: could not close STDOUT: %s\n", strerror(errno));
            exit(1); 
        } 
        fprintf(stderr,"Error: could not close file: %s\n", strerror(errno));
        exit(1); 
    }

    check = myfclose(readStream);

    if (check == -1){
        if (readStream->fd == STDIN_FILENO){
            fprintf(stderr,"Error: could not close STDIN: %s\n", strerror(errno));
            exit(1); 
        } 
        fprintf(stderr,"Error: could not close file: %s\n", strerror(errno));
        exit(1); 
    }

    end = clock();
    cpu_time_used = ((double) (end-start)) / CLOCKS_PER_SEC; 
    printf("%f\n", cpu_time_used);

    return 0;
}
