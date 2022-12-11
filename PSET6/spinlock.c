#include "tas.h"
#include "spinlock.h"

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