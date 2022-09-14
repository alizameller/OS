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
                write = 1; // a write stream is open
                break;
        }
    }

    if (!write){ // a write stream is not yet open 
        *writeStream = myfdopen(STDOUT_FILENO, O_WRONLY, 4096);
    }

    for(; optind < argc; optind++){ 
        printf("opening for reading: %s\n", argv[optind]);
        *readStream = myfopen(argv[optind], O_RDONLY, 4096);
        read = 1; // a read stream is open
    }

    if (!read){ // a read stream is not yet open
        *readStream = myfdopen(STDIN_FILENO, O_RDONLY, 4096);
    }
}

int main(int argc, char** argv){
    struct MYSTREAM* writeStream;
    struct MYSTREAM* readStream;

    set_streams(argc, argv, &writeStream, &readStream);

    int val;

    while((val = myfgetc(readStream)) != -1) { //check man pages for reaching end of file read

        if (val == '\t'){
            for (int i = 0; i < 4; i++){
                myfputc(' ', writeStream);
            }
            continue;
        }

        myfputc(val, writeStream);
    }

    myfclose(writeStream);
    myfclose(readStream);

    return 0;
}