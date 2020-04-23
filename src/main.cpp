#include "test_fine_grained_BST.h"

int main() {
    TestFineGrainedBST* testor = new TestFineGrainedBST();
    testor->test_all_basic();
    delete testor;
    return 0;
}