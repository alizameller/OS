#include "tas.h"
#include "spinlock.h"

void spin_init(struct spinlock *l) {
    if ((l = (struct spinlock*) mmap(l, sizeof(struct spinlock), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0)) == MAP_FAILED) {
        fprintf(stderr,"Error: could not mmap struct spinlock: %s\n", strerror(errno));
        exit(1);
    }
    l->numOps = l->currentHolder = l->primitiveLock = 0;
}

void spin_lock(struct spinlock *l) {
    while (tas(&(l->primitiveLock)) != 0); // while primitive lock is locked, try again
    l->currentHolder = getpid();
    (l->numOps)++;
}

void spin_unlock(struct spinlock *l) {
    l->currentHolder = 0;
    (l->numOps)++;
    l->primitiveLock = 0;
}