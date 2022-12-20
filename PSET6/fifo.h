#include "cv.h"
#define MYFIFO_BUFSIZ 1024


struct fifo {
    long buf[MYFIFO_BUFSIZ];
    int next_read, next_write, num_items;
    struct cv *full, *empty;
    struct spinlock* l;
};


/* Initialize the shared memory FIFO *f including any required underlying
* initializations (such as calling cv_init). The FIFO will have a static
* fifo length of MYFIFO_BUFSIZ elements. #define this in fifo.h.
* A value of 1K is reasonable.
*/
void fifo_init(struct fifo *f);


/* Enqueue the data word d into the FIFO, blocking unless and until the
* FIFO has room to accept it. (i.e. block until !full)
* Wake up a reader which was waiting for the FIFO to be non-empty
*/
void fifo_wr(struct fifo *f, unsigned long d);


/* Dequeue the next data word from the FIFO and return it. Block unless
* and until there are available words. (i.e. block until !empty)
* Wake up a writer which was waiting for the FIFO to be non-full
*/
unsigned long fifo_rd(struct fifo *f);