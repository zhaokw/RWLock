#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "rwlock.h"

//helpers
    static inline int
    cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *expire)
    {
        

        int r; 
        if (expire != NULL)  {
            r = pthread_cond_timedwait(c, m, expire);
        } else
            r = pthread_cond_wait(c, m);
        assert(r == 0 || r == ETIMEDOUT);
            return r;
    }

    //Initialise each pthread variable
    void initPthreads(rwl* l){
        l->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        pthread_cond_init (&l->wcon, NULL);
        pthread_cond_init (&l->rcon, NULL);
        pthread_mutex_init (l->mutex, NULL);
    }
//End: Helpers

void
rwl_init(rwl *l)
{
	l->readers = 0;
	l->writers = 0;
	l->rwaiters = 0; 
	l->wwaiters =0;
    initPthreads(l);
}

int
rwl_nwaiters(rwl *l) 
{
	return l->rwaiters + l->wwaiters;
}

int
rwl_rlock(rwl *l, const struct timespec *expire)
{
   pthread_mutex_lock(l->mutex);
    //Check if any writer or writer waiter is here, as writer has priority over reader
    while(l->writers | l->wwaiters){
        l->rwaiters++;
        int status = cond_timedwait(&l->rcon, l->mutex, expire);
        if(status == ETIMEDOUT){
            l->rwaiters--;
            pthread_mutex_unlock(l->mutex);
            return ETIMEDOUT;
        }else{
            l->rwaiters--;
            l->readers++;
            pthread_mutex_unlock(l->mutex);
            return 0;
        }
        
    }
         l->readers++;
        pthread_mutex_unlock(l->mutex);
        return 0;

}

//rwl_runlock unlocks the lock held in the "read" mode
void
rwl_runlock(rwl *l)
{
	pthread_mutex_lock(l->mutex);
	l->readers--;
    if(l->readers == 0)
	    pthread_cond_signal(&l->wcon);
	pthread_mutex_unlock(l->mutex);
	return;

}

int
rwl_wlock(rwl *l, const struct timespec *expire)
{							
    pthread_mutex_lock(l->mutex);
    printf("Mutex addr:%p\n", l->mutex);
    //Check if there is alreay another writer or reader
    while(l->writers | l->readers){
        l->wwaiters++;
        int status = cond_timedwait(&l->wcon, l->mutex, NULL);
        printf("1 Good\n");
        if(status == ETIMEDOUT){
            l->wwaiters--;
            pthread_mutex_unlock(l->mutex);
            printf("2 Good\n");
            return status;
        }
    }
    printf("3 Good\n");
    l->wwaiters--;
    l->writers++;
    pthread_mutex_unlock(l->mutex);
    return 0;
}


void
rwl_wunlock(rwl *l)
{
	pthread_mutex_lock(l->mutex);
	l->writers--;
    pthread_mutex_unlock(l->mutex);
	pthread_cond_signal(&l->wcon);
	pthread_cond_broadcast(&l->rcon);
	// printf("Writer Unlocked\n");
}
