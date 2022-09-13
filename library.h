#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

struct MYSTREAM {
    int fd;
    int bufsiz;
    char* buf;
    char* head;
    char* tail;
    int mode;
};

// insert description
struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz);

// insert description
struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz);

// insert description
int myfgetc(struct MYSTREAM *stream);

// insert description
int myfputc(int c,struct MYSTREAM *stream);

/*
myfclose: If the stream was opened for reading, myfclose simply calls close on the underlying file descriptor, returning 0 if this 
succeeds, or -1 if there is an error in close. Any characters that had been in the buffer but were not read with myfgetc are lost. 
After the close, call free to free the dynamically allocated memory. If the stream was opened for writing, you must first call write 
to flush the buffer, before you close the file descriptor, and a failure of either write or close must result in returning -1.
*/
int myfclose(struct MYSTREAM *stream);
