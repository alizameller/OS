#include "library.c"

// TODO
// Tab thing
// myfflush()
// buffer thing with graphs




// Helper function so that all four cases are accounted for
void set_streams(int argc, char** argv, struct MYSTREAM** writeStream, struct MYSTREAM** readStream){
    int opt;
    while((opt = getopt(argc, argv, "o:")) != -1) {
            switch (opt) { 
                case 'o': 
                    printf("opening for writing: %s\n", optarg); 
                    *writeStream = myfopen(optarg, O_WRONLY, 4096);
                    break;

                default:
                    *writeStream = myfdopen(STDOUT_FILENO, O_WRONLY, 4096);
            }
        }

    if (optind == argc){
        *readStream = myfdopen(STDIN_FILENO, O_RDONLY, 4096);
    }

    for(; optind < argc; optind++){ 
        printf("opening for reading: %s\n", argv[optind]);
        *readStream = myfopen(argv[optind], O_RDONLY, 4096);
    }

}

int main(int argc, char** argv){
    int i; 
    struct MYSTREAM* writeStream;
    struct MYSTREAM* readStream;

    set_streams(argc, argv, &writeStream, &readStream);


    int val;
    while((val = myfgetc(readStream)) != -1) {
        myfputc(val, writeStream);
    }

    myfclose(writeStream);
    myfclose(readStream);

    return 0;
}