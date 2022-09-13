#include "library.h"

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz){
    if ((mode != O_RDONLY && mode != O_WRONLY) || bufsiz <= 0){
        errno = EINVAL;

        return NULL;
    } 
    
    struct MYSTREAM *stream = (struct MYSTREAM*) malloc(sizeof(struct MYSTREAM));
    char* buffer = (char*) malloc(sizeof(char)*bufsiz);
    if (stream == NULL || buffer == NULL){
        errno = ENOMEM;

        return NULL;
    }

    if (mode == O_WRONLY){
        mode = O_WRONLY | O_CREAT | O_TRUNC;
    } 
    int fd = open(pathname, mode, S_IRWXU | S_IRWXG | S_IRWXO);
    
    if (fd == -1){
        return NULL;
    } 

    stream->mode = mode;
    stream->fd = fd;
    stream->bufsiz = bufsiz;
    stream->buf = stream->head = stream->tail = buffer;

    return stream; 
}

struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz){
    if (filedesc < 0){
        return NULL;
    } 

    if ((mode != O_RDONLY && mode != O_WRONLY) || bufsiz <= 0){
        errno = EINVAL;

        return NULL;
    } 

    struct MYSTREAM *stream = (struct MYSTREAM*) malloc(sizeof(struct MYSTREAM));
    char* buffer = (char*) malloc(sizeof(char)*bufsiz);
    if (stream == NULL || buffer == NULL){
        errno = ENOMEM;

        return NULL;
    }

    stream->mode = mode;
    stream->fd = filedesc;
    stream->bufsiz = bufsiz;
    stream->buf = stream->head = stream->tail = buffer;

    return stream; 
}


int myfgetc(struct MYSTREAM *stream){
    if (stream->buf == stream->tail) { //if buffer is empty or full/exhausted
        ssize_t size = read(stream->fd, stream->head, stream->bufsiz);

        if (size == 0){
            errno = 0;
            return -1;
        } else if (size == -1){
            return -1;
        }

        stream->tail = stream->head + size;
        stream->buf = stream->head;

        char character = *(stream->buf);
        (stream->buf)++;
        return (int) character;
    }

    char character = *(stream->buf);
    (stream->buf)++;
    return (int) character;
}

int myfputc(int c,struct MYSTREAM *stream){
    if (stream->buf == stream->tail){  //if buffer is full or empty
        if (stream->buf != stream->head){ //if buffer is full
            ssize_t size = write(stream->fd, stream->buf, stream->bufsiz);
        
            if (size <= 0){
                return -1; 
            }
            
            stream->buf = stream->head;
            *(stream->buf) = (char) c;
            (stream->buf)++; 

            return c; 
        } // if buffer is empty
        stream->tail = stream->head + stream->bufsiz;
        *(stream->buf) = (char) c;
        (stream->buf)++; 

        return c; 
    }

    *(stream->buf) = (char) c;
    (stream->buf)++; 

    return c; 
}

int myfclose(struct MYSTREAM *stream){
    if(stream->mode == O_RDONLY){ //how do i determine if its rd_only
        int val = close(stream->fd); // will return 0 upon success or -1 upon failure
        stream->buf = stream->head;
        free(stream->buf);
        free(stream); 

        return val;
    }
    ssize_t size = write(stream->fd, stream->buf, stream->bufsiz);

        if (size <= 0){
            return -1; 
        }

    int val = close(stream->fd); //will return 0 upon success or -1 upon failure
    stream->buf = stream->head;
    free(stream->buf);
    free(stream); 

    return val;
}
