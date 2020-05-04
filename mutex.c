#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "kthread.h" 


static int nextmid = 1;

struct {
	struct spinlock lock;
	struct kthread_mutex_t mtx[MAX_MUTEXES];
} mtx_array;

void mutex_lock_init(){
    initlock(&mtx_array.lock, "mutex_array");
}

int kthread_mutex_alloc(){
	struct kthread_mutex_t *m;
	acquire(&mtx_array.lock);
  	for(m = mtx_array.mtx; m < &mtx_array.mtx[MAX_MUTEXES]; m++)
  		if(m->state == NOTUSED || m->owner->state == ZOMBIE || m->owner->state == UNUSED) // allocate if the mutex is free or the owner is dead.
  			goto found;
  	release(&mtx_array.lock);
	return -1;

	found:
		m->state = UNLOCKED;
		m->mid = nextmid++;
		m->owner = mythread();
		release(&mtx_array.lock);
		return m->mid;
}

int kthread_mutex_dealloc(int mutex_id){
	struct kthread_mutex_t *m;
	acquire(&mtx_array.lock);
  	for(m = mtx_array.mtx; m < &mtx_array.mtx[MAX_MUTEXES]; m++)
  		if(m->mid == mutex_id)
  			goto found;
  	release(&mtx_array.lock);
	return -1;

	found:
        if(m->state == LOCKED){
  			release(&mtx_array.lock);
  			return -1;
  		}
		m->state = NOTUSED;
		m->mid = 0;
		m->owner = 0;
		release(&mtx_array.lock);
		return 0;
}

int kthread_mutex_lock(int mutex_id) {
    struct kthread_mutex_t *m;
	acquire(&mtx_array.lock);
  	for(m = mtx_array.mtx; m < &mtx_array.mtx[MAX_MUTEXES]; m++){
		if(m->mid == mutex_id)
  			goto found;
	  }
  	release(&mtx_array.lock);
	return -1;

	found:
		if(m->state == LOCKED && m->owner == mythread()){
	  	  	release(&mtx_array.lock);
			return -1;
  		}
		while(xchg(&m->state, LOCKED) == LOCKED){  ///while the return value is LOCKED there is a thread that acquired the mutex
			sleep(m, &mtx_array.lock);
  		}
		m->state = LOCKED;
		m->owner = mythread();
  	release(&mtx_array.lock);
	return 0;
}

int kthread_mutex_unlock(int mutex_id) {
    struct kthread_mutex_t *m;
	acquire(&mtx_array.lock);
  	for(m = mtx_array.mtx; m < &mtx_array.mtx[MAX_MUTEXES]; m++)
  		if(m->mid == mutex_id && m->owner == mythread()) // unlock iff the owner thread is unlocking.
  			goto found;
  	release(&mtx_array.lock);
	return -1;

	found:
        if(m->state == UNLOCKED){
	  	  	release(&mtx_array.lock);
			return -1;
  		}
        m->state = UNLOCKED;
		m->owner = 0;
        release(&mtx_array.lock);
        wakeup(m);
    return 0;
}
