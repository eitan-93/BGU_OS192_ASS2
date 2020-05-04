#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"


int kthread_create(void(*start_func)(), void* stack){
  struct thread *t;
  struct proc *p = myproc();  
  t = allocThread(p);
  if(t==0)
    return -1;
  *t->tf = *mythread()->tf; // initialize tf with non-garbage values 
  t->tf->eip = (uint)start_func; 
  t->tf->esp = (uint)(stack); //set the stack pointer - setting the stack as is, adding MAX_STACK_SIZE to the pointer returned from malloc.

  acquire(&p->lock);
  t->state = RUNNABLE;
  release(&p->lock);
  
  return t->tid; 
}

void wakeupThread(void *chan){ 

  struct thread *currth = mythread();
  struct thread *t;
  for(t = currth->tproc->pthreads; t < &currth->tproc->pthreads[NTHREAD]; t++)
    if(t->state == SLEEPING && t->chan == chan){
      t->state = RUNNABLE;
    }
}

int kthread_id(void){
  if(mythread()->tid <= 0)
    return -1;
  return mythread()->tid;
}

void kthread_exit(void){
  struct thread *t;
  struct thread *currth = mythread();
  lockPTable();
  currth->state = ZOMBIE;

  int threads_num = 0;
  for(t = currth->tproc->pthreads; t < &currth->tproc->pthreads[NTHREAD] ; t++){ // if thread_num increases we ha live threads, else we call exit().
      if(t->state != UNUSED && t->state != ZOMBIE){
        threads_num++;
        }
  }
  wakeupThread(currth);
  
  if(threads_num == 0){
    unlockPTable();
    exit();
  }

  sched(); //should not return from here
}


void single_waiting_th(void *chan){
  //int sum = 0;
  struct thread *t;
  struct proc *p = myproc();
    for(t = p->pthreads ; t < &p->pthreads[NTHREAD] ; t++)
      if(t->state == SLEEPING && t->chan == chan){
        t->chan = t;
        t->state = RUNNABLE; 
      }
    
}

int kthread_join(int thread_id){
  struct thread *t;
  struct proc *currproc = myproc();
  acquire(&currproc->lock);
  for(t = currproc->pthreads; t < &currproc->pthreads[NTHREAD]; t++)
    if(t->tid == thread_id)
      break;
  if(t == &currproc->pthreads[NTHREAD]){
    // thread_id not found 
    release(&currproc->lock);
    return -1;   
  }
  if(t->state == ZOMBIE){
    //wakeup(t);
    //if(single_waiting_th(t)){
      single_waiting_th(t);
      kfree(t->kstack);
      t->kstack = 0;
      t->state = UNUSED;
    release(&currproc->lock);
    return 0;
  }
  else if(t->state == UNUSED){
    release(&currproc->lock);
    return -1;
  }
  
  else{ // rest of states, not UNUSED or ZOMBIE.
    
    while(t->state != ZOMBIE){
      sleep(t, &currproc->lock);
    }
    if(t->state == UNUSED)
      panic("kthread_join: thread become unused");
    
    //if(){
      single_waiting_th(t);
      kfree(t->kstack);
      t->kstack = 0;
      t->state = UNUSED;
    //}
    //else t->killed = 1;
    release(&currproc->lock);
    return 0;
  }
}