#include "test_fine_grained_BST.h"

int main() {
    TestFineGrainedBST* testor = new TestFineGrainedBST();
//    testor->test_all_basic();
    testor->test_all_multi(4, 4);
    delete testor;
    return 0;
}