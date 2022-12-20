#include "fifo.h"

/* Initialize the shared memory FIFO *f including any required underlying
* initializations (such as calling cv_init). The FIFO will have a static
* fifo length of MYFIFO_BUFSIZ elements. #define this in fifo.h.
* A value of 1K is reasonable.
*/
void fifo_init(struct fifo *f) {
    if ((f->full = (struct cv*) mmap(NULL, sizeof(struct cv), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED
        || (f->empty = (struct cv*) mmap(NULL, sizeof(struct cv), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap struct cv: %s\n", strerror(errno));
        exit(1);
    }
    cv_init(f->full), cv_init(f->empty);
    
    f->next_read = f->next_write = f->num_items = 0;

    if ((f->l = (struct spinlock*) mmap(NULL, sizeof(struct spinlock), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap struct spinlock: %s\n", strerror(errno));
        exit(1);
    }
    spin_init(f->l);
}


/* Enqueue the data word d into the FIFO, blocking unless and until the
* FIFO has room to accept it. (i.e. block until !full)
* Wake up a reader which was waiting for the FIFO to be non-empty
*/
void fifo_wr(struct fifo *f, unsigned long d) {
    spin_lock(f->l);
    while (f->num_items >= MYFIFO_BUFSIZ) {
        cv_wait(f->full, f->l);
    }
    f->next_write %= MYFIFO_BUFSIZ;
    f->buf[f->next_write++] = d;
    f->num_items++;
    cv_signal(f->empty);
    spin_unlock(f->l);
}

/* Dequeue the next data word from the FIFO and return it. Block unless
* and until there are available words. (i.e. block until !empty)
* Wake up a writer which was waiting for the FIFO to be non-full
*/
unsigned long fifo_rd(struct fifo *f) {
    spin_lock(f->l);
    while(f->num_items <= 0) {
        cv_wait(f->empty, f->l);
    }
    f->next_read %= MYFIFO_BUFSIZ;
    unsigned long ret = f->buf[f->next_read++];
    f->num_items--;
    cv_signal(f->full);
    spin_unlock(f->l);
    return ret;
}