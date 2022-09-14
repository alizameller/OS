#include "library.c"

// TODO
// Tab thing
// myfflush()
// buffer thing with graphs


// Helper function so that all four cases are accounted for
void set_streams(int argc, char** argv, struct MYSTREAM** writeStream, struct MYSTREAM** readStream){
    int opt;
    int write = 0;
    int read = 0;

    while((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) { 
            case 'o': 
                printf("opening for writing: %s\n", optarg); 
                *writeStream = myfopen(optarg, O_WRONLY, 4096);
                
                if (*writeStream == NULL){
                fprintf(stderr,"Error: could not open %s for writing\n", optarg, strerror(errno));
                exit(1); 
            }
                write = 1; // a write stream is open
                break;
        }
    }

    if (!write){ // a write stream is not yet open 
        *writeStream = myfdopen(STDOUT_FILENO, O_WRONLY, 4096);
    }

    if (*writeStream == NULL){
        fprintf(stderr,"Error: could not open STDOUT for writing\n", strerror(errno));
        exit(1); 
    }

    for(; optind < argc; optind++){ 
        printf("opening for reading: %s\n", argv[optind]);
        *readStream = myfopen(argv[optind], O_RDONLY, 4096);
        
        if (*readStream == NULL){
        fprintf(stderr,"Error: could not open %s for reading\n", argv[optind], strerror(errno));
        exit(1); 
    }
        read = 1; // a read stream is open
    }

    if (!read){ // a read stream is not yet open
        *readStream = myfdopen(STDIN_FILENO, O_RDONLY, 4096);
    }

    if (*readStream == NULL){
        fprintf(stderr,"Error: could not open STDOUT for reading\n", strerror(errno));
        exit(1); 
    }

}

int main(int argc, char** argv){
    struct MYSTREAM* writeStream;
    struct MYSTREAM* readStream;

    set_streams(argc, argv, &writeStream, &readStream);

    int val = 0;
    int check = 0;

    while((val = myfgetc(readStream)) != 0) { //check man pages for reaching end of file read
     
        if (val == -1) { // this will never be hit help
            // OUTPUT FILE?!?!
            fprintf(stderr,"Error: could not get char %c from STDIN\n", check, strerror(errno));
            exit(1); 
        }

        if (val == '\t'){
            for (int i = 0; i < 4; i++){
                check = myfputc(' ', writeStream);

                if (check == -1) {
                    // OUTPUT FILE?!?!
                    fprintf(stderr,"Error: could not put char %c in STDOUT\n", check, strerror(errno));
                    exit(1); 
                }
            }
        
            continue;
        }

        check = myfputc(val, writeStream);

        if (check == -1){
        fprintf(stderr,"Error: could not put char %c in STDOUT\n", check, strerror(errno));
        exit(1); 
        }
    }

    check = myfclose(writeStream);

    if (check == -1){
        fprintf(stderr,"Error: could not close STDOUT\n", strerror(errno));
        exit(1); 
    }

    check = myfclose(readStream);

    if (check == -1){
        fprintf(stderr,"Error: could not close STDIN\n", strerror(errno));
        exit(1); 
    }

    return 0;
}