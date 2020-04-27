#include "test_BST.h"
//#include "lock_free_BST.h"

int main() {
    TestBST* testor = new TestBST();
//    testor->test_fine_grained_BST(4, 4);
    testor->test_lock_free_BST(4, 4);
    delete testor;

//    LockFreeBST* bst = new LockFreeBST();
//    bst->search(1);
//    bst->search(2);
//    bst->insert(1);
//    bst->insert(3);
//    bst->insert(2);
//    bst->insert(2);
//    bst->insert(2);
//    bst->search(1);
    return 0;
}