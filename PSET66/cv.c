#include "cv.h"

void handler(int signum) {
    return; 
}

/* Initialize any internal data structures in cv so that it is ready for
* use. The initial condition is that nobody is waiting for this cv.
* You can probably arrange your struct cv so that all-0 bytes is
* the initialization condition.
*/
void cv_init(struct cv *cv) {
    memset(cv, 0, sizeof(struct cv));
}

/* This must be called with the spinlock mutex held by the caller (otherwise
* results will be undefined). Atomically record within the internals
* of *cv that the caller is going to sleep (the wait list), release the
* mutex, and go to sleep (see text below). The wait list is of static size
* CV_MAXPROC waiters. After waking up, re-acquire the mutex
* before returning to the caller
*/
void cv_wait(struct cv *cv, struct spinlock *mutex) {
    //spin_lock(&(cv->intern_lock));
    sigset_t mask, oldmask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    signal(SIGUSR1, handler);

    cv->sleepers[cv->num_sleepers++] = getpid();

    spin_unlock(mutex);
    //spin_unlock(&(cv->intern_lock));

    sigsuspend(&oldmask);

    sigprocmask(SIG_SETMASK,&oldmask,NULL);
    signal(SIGUSR1,SIG_DFL);
    spin_lock(mutex);
    return;
}

/* Wake up any and all waiters (sleepers) on this cv. If there are no waiters
* the call has no effect and is not "remembered" for the next time that
* someone calls cv_wait. cv_broadcast must be called with the same mutex
* held that protects cv_wait, as discussed in lecture notes under "Lost
* Wakup", but note that cv_broadcast does not take a mutex as a parameter.
* Return value: the number of sleepers that were awoken.
*/
int cv_broadcast(struct cv *cv) {
    //spin_lock(&(cv->intern_lock));
    int temp = cv->num_sleepers;
    for (int i = 0; i < cv->num_sleepers; i++) {
        kill(cv->sleepers[i], SIGUSR1);
    }
    cv->num_sleepers = 0;
    //spin_unlock(&(cv->intern_lock));
    return temp;
}

/* Exactly the same as cv_broadcast except at most one sleeper is awoken.
* Your choice how to pick which one if more than one candidate
*/
int cv_signal(struct cv *cv) {
    //spin_lock(&(cv->intern_lock));
    if (!(cv->num_sleepers)) {
        spin_unlock(&(cv->intern_lock));
        return 0;
    }
    kill(cv->sleepers[--cv->num_sleepers], SIGUSR1);
    //spin_unlock(&(cv->intern_lock));
    return 1;
}
