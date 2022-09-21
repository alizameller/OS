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

// insert description
int myfflush (struct MYSTREAM *stream);

// insert description
<<<<<<< HEAD
int myfclose(struct MYSTREAM *stream);
=======
int myfclose(struct MYSTREAM *stream);
>>>>>>> 02ebfa0039d49eff85bf6f080dea83eba2a1a78a
