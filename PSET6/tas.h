#ifndef __TAS_H
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

int tas(volatile char *lock);

#define __TAS_H
#endif
