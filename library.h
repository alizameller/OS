#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

struct MYSTREAM {
    int fd;
    int bufsiz;
    char* buf;
    char* head;
    char* tail;
    int mode;
};

/*
myfopen takes a pathname and a mode which is either O_RDONLY or O_WRONLY (the O_RDWR mode is not supported). 
In the WRONLY case, the behavior is the same as fopen: open the file for truncation, creation if it doesn’t already exist, 
and with a permissions mode of 0777. The bufsiz parameter is the size of the character buffer, in bytes. 
The myfopen function should allocate a struct MYSTREAM using malloc and the return value from the function 
is a pointer to that, or NULL upon failure. Therefore the semantics are the same as fopen. The layout of struct 
MYSTREAM is up to you as this is an opaque or "private" (if C had such a notion) data structure for the library’s use only. 
You must also allocate the buffer, either as part of MYSTREAM by using the C trick of a structure whose last member is an 
array of indefinite size, or by a second malloc. Obviously, myfopen will call the actual system call open, and must save the 
file descriptor within its struct MYSTREAM. Upon error, return NULL and do not print out any error messages (this is up to 
the application which is using your library -- note that the errno code will still be set.) If the mode or bufsiz parameter 
is invalid, you may set errno to EINVAL and return NULL immediately. If malloc fails, set errno to ENOMEM.
*/
struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz);

/*
myfdopen works the same as myfopen except it takes a file descriptor that is already open, e.g. the result of the application 
calling open, or one of the 3 standard file descriptors (0,1,2) that is open when any program starts. It is assumed that the 
file descriptor was opened with a mode that is identical to or compatible with mode.
*/
struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz);

/*
myfgetc returns a character from the buffer associated with stream. If the buffer is empty (the buffer will always be empty on 
the first call after opening) then myfgetc must call read to obtain a bufsiz worth of bytes. This doesn’t mean that bufsiz bytes 
will actually be read, so you must keep track of how many bytes are actually in the buffer! Return the character as an int. If 
the read system call returns 0 (indicating an EOF condition) then return the value -1 (which can not be a valid character) and 
make sure errno=0. If the read system call returns an error, also return -1, and it is up to the application to read errno and 
make a valid error report. So in summary, we see that myfgetc is almost identical to the standard library function fgetc.
*/
int myfgetc(struct MYSTREAM *stream);

/*
myfputc takes the supplied character and places it into the buffer. If the buffer is now full (the count of valid characters in 
the buffer is equal to the bufsiz parameter from the open) then call the write system call to flush the buffer. If write returns 
an error, return the value -1 (and note that errno will contain the error code for the application). If write returns 0, which 
will not happen under ordinary circumstances, treat this as an error. If write returns less than bufsiz, that is a partial write, 
see the section Extra Credit for more information. If the buffer was not full, and/or the write system call succeeded, the return 
value of myfputc is the same as the parameter c. These semantics are the same as fputc except only the file buffered mode is supported.
*/
int myfputc(int c,struct MYSTREAM *stream);

/*
myfclose: If the stream was opened for reading, myfclose simply calls close on the underlying file descriptor, returning 0 if this 
succeeds, or -1 if there is an error in close. Any characters that had been in the buffer but were not read with myfgetc are lost. 
After the close, call free to free the dynamically allocated memory. If the stream was opened for writing, you must first call write 
to flush the buffer, before you close the file descriptor, and a failure of either write or close must result in returning -1.
*/
int myfclose(struct MYSTREAM *stream);
