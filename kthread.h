#ifndef KTHREAD_H
#define KTHREAD_H
#define MAX_STACK_SIZE 4000
#define MAX_MUTEXES 64

/********************************
        The API of the KLT package
 ********************************/

struct thread* mythread(void);

enum tpstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
struct thread {
  char *kstack;                // Bottom of kernel stack for this thread
  enum tpstate state;          // thread state
  int tid;		                 // thread ID
  struct proc *tproc;	         // containing process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  char name[16];               // thread name (debugging)
  
};


int kthread_create(void (*start_func)(), void* stack);
int kthread_id();
void kthread_exit();
int kthread_join(int thread_id);


//task 3

enum mstate {NOTUSED = 0, UNLOCKED = 1, LOCKED = 2};
struct kthread_mutex_t {
  enum mstate state;     	// mutex state
  int mid;					        // mutex ID
  struct thread * owner;
};

void mutex_lock_init();
int kthread_mutex_alloc();
int kthread_mutex_dealloc(int mutex_id);
int kthread_mutex_lock(int mutex_id);
int kthread_mutex_unlock(int mutex_id);

#endif