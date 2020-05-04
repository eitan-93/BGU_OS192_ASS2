#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "kthread.h"



int sys_kthread_mutex_alloc(void){
  return kthread_mutex_alloc();
}

int sys_kthread_mutex_dealloc(void){
  int mid;
  if(argint(0, &mid) < 0)
    return -1;
  return kthread_mutex_dealloc(mid);
}

int sys_kthread_mutex_lock(void){
  int mid;
  if(argint(0, &mid) < 0)
    return -1;
  return kthread_mutex_lock(mid);
}

int sys_kthread_mutex_unlock(void){
  int mid;
  if(argint(0, &mid) < 0)
    return -1;
  return kthread_mutex_unlock(mid);
}
