#include "types.h"
/*#include "stat.h"*/
#include "user.h"
//#include "kthread.h"
#include "tournament_tree.h"

int n = 100;
void countDown() {
    if (n <= 1)
		return;
    n--;
	countDown();
    n--;
	countDown();
    kthread_exit();
}

int k = 100;
void countDown1() {
    if (k <= 1)
		return;
    k--;
	countDown1();
    kthread_exit();
}

int mutex_id = 0;
void alloc_test(){
    if((mutex_id = kthread_mutex_alloc()) < 0 )
        printf(1, "alloc failed\n");
    else  printf(1, "alloc passed\n");
}

void dealloc_test(){
    if(kthread_mutex_dealloc(mutex_id) < 0)
        printf(1, "dealloc test passed\n");
    else  printf(1, "dealloc test failed\n");
    
}

void lock_test(){
    if(kthread_mutex_lock(mutex_id) < 0)
        printf(1, "lock failed\n");
    else printf(1, "lock passed\n");
}
void sec_thread_lock_test(){
    if(kthread_mutex_lock(mutex_id) < 0)
        printf(1, "lock failed\n");
    kthread_exit();
}
void lock_and_unlock_test(){
    if(kthread_mutex_lock(mutex_id) < 0)
        printf(1, "lock failed\n");
    kthread_mutex_unlock(mutex_id);
    printf(1, "lock passed\n");
    kthread_exit();
}
void unlock_test(){
    if(kthread_mutex_unlock(mutex_id) < 0)
        printf(1, "unlock test failed\n");
    else {
        printf(1, "unlock test passed\n");
    }
}


struct trnmnt_tree* t;
int j = 0;
int in = 0;

void print_a_number(){
    int id = in++;

    if(trnmnt_tree_acquire(t,id) == -1)
        printf(1,"problem 1\n");
 
    printf(1,"number is %d and %d\n",id,j++);
    //sleep(8);
    
    trnmnt_tree_release (t, id);
    //sleep(10);
    //kthread_join(kthread_id());
    kthread_exit();
    //exit();
}

void tree_test(){
    //sleep(20);
    int thread_id[32];
    int depth = 4;
    char *stack;
    t = trnmnt_tree_alloc (depth);

    for (int i = 0; i < 16; i++ ){
        stack = ((char *) malloc(500 * sizeof(char))) + 500;
        thread_id[i] = kthread_create(&print_a_number,stack);
    }
    for (int i = 0; i < 16; i++ ){
        int result = kthread_join(thread_id[i]);
        if(result < 0){
            printf(1,"Finished joing thread %d\n",i); // the last one should fail
        }
    }
    sleep(200);
    trnmnt_tree_dealloc (t);
    
}


int main(void){

    char *stack1 = malloc(4000) + MAX_STACK_SIZE;
    char *stack2 = malloc(4000) + MAX_STACK_SIZE;
    int first = 0;
    int second = 0;
    printf(1, "-----start threads test----\n");
    first = kthread_create(&countDown,stack1);
    second = kthread_create(&countDown1,stack2);
    
   
    if(kthread_join(first) < 0) {
        printf(1, "join failed\n\n");
    }
    if(kthread_join(second) < 0) {
        printf(1, "join failed\n");
    }
    printf(1, "-----end threads test----\n\n");
    printf(1, "-----start mutex test----\n");
    alloc_test();
    lock_test();
    stack1 = malloc(4000);
    stack2 = malloc(4000);
    first = kthread_create(&lock_and_unlock_test,stack1);
    dealloc_test();
    unlock_test();
    if(kthread_join(first) < 0) {
        printf(1, "join failed\n");
    }
    second = kthread_create(&sec_thread_lock_test,stack2);
    if(kthread_join(second) < 0) {
        printf(1, "join failed\n");
    }
    dealloc_test();
    printf(1, "-----end mutex test----\n\n");
    printf(1, "-----start tree test----\n");
    tree_test();
    printf(1, "-----start exec test----\n");
    char * command;
    char *args[4];
    args[0] = "ls";
    args[1] = "";
    args[2] = 0;
    command = "ls";
    exec(command,args);
    printf(1, "shouldn't be here\n");
    //sleep(400);
    
  exit();
}
