 #include "types.h"
 #include "user.h"
 #include "tournament_tree.h"


int size_of_tree;
int incr = 0;
int please_dont_dealloc = 0;
/*
* creating an array of leafs to allow the threads to access the tree.
*/

struct trnmnt_tree* set_right(int index,struct trnmnt_tree* tree){
    if(((2*index)+2)<=(size_of_tree-2 ))
        return &tree[(2*index)+2];
    return 0;
}

struct trnmnt_tree* set_left(int index,struct trnmnt_tree* tree){
    if(((2*index)+1)<=(size_of_tree-2 ))
        return &tree[(2*index) + 1];
    return 0;
}

struct trnmnt_tree* set_parent(int index,struct trnmnt_tree* tree){
    if(index>0 && ((index-1)/2)<=(size_of_tree-2 )) //root is 0.
        return &tree[(index-1)/2];
    return 0;
}

struct trnmnt_tree* trnmnt_tree_alloc (int depth){
    size_of_tree = 1 << depth;
    struct trnmnt_tree* tree = (struct trnmnt_tree*) malloc((size_of_tree - 1)*(sizeof (struct trnmnt_tree)));
    int i;
    for (i = 0; i < size_of_tree - 1; i++){
        tree[i].acquirerid = -666;
        tree[i].rootmtxid = kthread_mutex_alloc();
        tree[i].right = set_right(i,tree);
        tree[i].left = set_left(i,tree);
        tree[i].parent = set_parent(i,tree);
    }
    incr = 0;
    return tree;
}

/*
* check whether the tree is empty - before deallocating.
*/
int free_tree(struct trnmnt_tree* tree){
    for (int i=0; i<size_of_tree-1; i++) {
        if (tree[i].acquirerid != -666) {
            return -1;
        }
    }
    return 0;
}

int trnmnt_tree_dealloc (struct trnmnt_tree* tree){
    if((tree==0 || free_tree(tree) == -1) || please_dont_dealloc) {
        //printf(1,"fail in dealloc, the tree is not empty\n");
        return -1;
    }
    int i;
    for (i = 0; i < size_of_tree -1; i++){
        if(kthread_mutex_dealloc(tree[i].rootmtxid) == -1){
            //printf(1,"fail in mutex %d dealloc by thread %d\n",tree[i].rootmtxid,tree[i].acquirerid);
            return -1;
        }
    }
    free(tree);
    tree=0;
    return 0;
}

int trnmnt_tree_acquire_wrapped (struct trnmnt_tree* tree, int ID){
    please_dont_dealloc++;
    if(kthread_mutex_lock(tree->rootmtxid) == -1){
        return -1;
    }   
    please_dont_dealloc--;   
    tree->acquirerid = ID;   
    if (tree->parent != 0) {
        return trnmnt_tree_acquire_wrapped (tree->parent, ID);
    }
    return ID;
}

int trnmnt_tree_acquire(struct trnmnt_tree* tree, int ID){
    int b = ID/2 + (size_of_tree-1)/2;
    //printf(1,"locking mutex %d with ID %d in leafs %d\n",tree->rootmtxid,ID,b);
    sleep(3);
    return trnmnt_tree_acquire_wrapped(&tree[b],ID);
}

int trnmnt_tree_release (struct trnmnt_tree* tree, int ID){ 
    if (tree->acquirerid == ID || tree->acquirerid == -666) {
        if(kthread_mutex_unlock(tree->rootmtxid) == -1){
            //printf(1,"thread %d try to release mutex %d failer\n", ID, tree->rootmtxid);
            return -1;
        }
        tree->acquirerid = -666;
        if(tree->left->acquirerid == ID)
            trnmnt_tree_release (tree->left, ID);
        if(tree->right->acquirerid == ID)
            trnmnt_tree_release (tree->right, ID);
        //printf(1,"in release : id released %d\n");
        return ID;
    }//else printf(1,"in release : the ID of the node %d != %d is try to release mutex %d failer\n",tree->acquirerid , ID, tree->rootmtxid);
    return -1;
}
