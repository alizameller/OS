#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include "tas.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <ctype.h> 
#include <limits.h>

void spin_lock(struct spinlock *l);

void spin_unlock(struct spinlock *l);

struct spinlock { 
    char primitiveLock;
    pid_t currentHolder;
    int numOps;
};

#endif // _SPINLOCK_H