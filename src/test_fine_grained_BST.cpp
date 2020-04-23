//
// Created by Leonard on 2020/4/23.
//

#include "test_fine_grained_BST.h"

bool TestFineGrainedBST::instantiateBST(vector<int> v) {
    bst->reinitialize();
    for(int element : v) {
        if(!bst->insert(element)) return false;
    }
    return true;
}


bool TestFineGrainedBST::compare(vector<int> vec) {
    vector<int> bst_vec = bst->trans2vec();
    return bst_vec == vec;
}


void TestFineGrainedBST::printResult(string test_name, bool succ) {
    cout << "Test " << test_name;
    if(succ) cout << " PASSED" << endl;
    else cout << "FAILED" << endl;
}


void TestFineGrainedBST::test_all_basic() {
    printResult("TEST_SEARCH_1", test_search_1());
    printResult("TEST_SEARCH_2", test_search_2());
    printResult("TEST_SEARCH_3", test_search_3());
    printResult("TEST_INSERT_1", test_insert_1());
    printResult("TEST_INSERT_2", test_insert_2());
    printResult("TEST_INSERT_3", test_insert_3());
    printResult("TEST_DELETE_1", test_delete_1());
    printResult("TEST_DELETE_2", test_delete_2());
    printResult("TEST_DELETE_3", test_delete_3());
    printResult("TEST_DELETE_4", test_delete_4());
    printResult("TEST_COMBO_1", test_combo_1());
    printResult("TEST_COMBO_2", test_combo_2());
}


bool TestFineGrainedBST::test_search_1() {
    if(!instantiateBST(v1)) return false;
    return bst->search(16);
}


bool TestFineGrainedBST::test_search_2() {
    if(!instantiateBST(v1)) return false;
    return bst->search(14);
}


bool TestFineGrainedBST::test_search_3() {
    if(!instantiateBST(v1)) return false;
    return !bst->search(10);
}


bool TestFineGrainedBST::test_insert_1() {
    if(!instantiateBST(v1)) return false;
    return compare(v1_pre_trav);
}


bool TestFineGrainedBST::test_insert_2() {
    if(!instantiateBST(v1)) return false;
    if(!bst->insert(10)) return false;
    return compare({16, 12, 7, 5, 11, 8, 10, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
}


bool TestFineGrainedBST::test_insert_3() {
    if(!instantiateBST(v1)) return false;
    return !bst->insert(36);
}


bool TestFineGrainedBST::test_delete_1() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(56)) return false;
    return compare({16, 12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55});
}


bool TestFineGrainedBST::test_delete_2() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(88)) return false;
    return compare({16, 12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 55, 56});
}


bool TestFineGrainedBST::test_delete_3() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(12)) return false;
    return compare({16, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
}


bool TestFineGrainedBST::test_delete_4() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(16)) return false;
    return compare({12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
}


bool TestFineGrainedBST::test_combo_1() {
    if(!instantiateBST(v1)) return false;
    if(!bst->insert(2)) return false;
    if(!bst->insert(1)) return false;
    if(!bst->insert(3)) return false;
    if(!bst->search(3)) return false;
    if(!bst->remove(2)) return false;
    if(!bst->remove(1)) return false;
    if(!bst->remove(3)) return false;
    return compare(v1_pre_trav);
}


bool TestFineGrainedBST::test_combo_2() {
    if(!instantiateBST(v1)) return false;
    if(!bst->search(33)) return false;
    if(!bst->remove(33)) return false;
    if(bst->search(33)) return false;
    if(!bst->remove(32)) return false;
    if(!bst->remove(36)) return false;
    if(!bst->remove(34)) return false;
    if(!bst->remove(88)) return false;
    if(!bst->insert(54)) return false;
    return compare({16, 12, 7, 5, 11, 8, 13, 15, 14, 55, 54, 56});
}

