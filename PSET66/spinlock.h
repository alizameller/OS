#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct spinlock { 
    char primitiveLock;
    pid_t currentHolder;
    int numOps;
};

void spin_init(struct spinlock *l);

void spin_lock(struct spinlock *l);

void spin_unlock(struct spinlock *l);

#endif // _SPINLOCK_H