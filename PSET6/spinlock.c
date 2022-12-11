#include "spinlock.h"

void spin_lock(struct spinlock *l) {
    while (tas(&(l->primitiveLock))); // while primitive lock is locked, try again
    l->currentHolder = getpid(); 
    l->numOps = l->numOps++; 
}

void spin_unlock(struct spinlock *l) {
    l->primitiveLock = 0;
    l->currentHolder = getpid(); 
    l->numOps = l->numOps++; 
}
