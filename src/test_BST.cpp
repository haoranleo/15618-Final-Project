//
// Created by Leonard on 2020/4/23.
//

#include "test_BST.h"

void TestBST::test_fine_grained_BST(unsigned int thread_num, unsigned int ops_num) {
    bst = new FineGrainedBST();
    test_all(thread_num, ops_num);
}


void TestBST::test_lock_free_BST(unsigned int thread_num, unsigned int ops_num) {
    bst = new LockFreeBST();
    test_all(thread_num, ops_num);
}


bool TestBST::instantiateBST(vector<int> v) {
    bst->reinitialize();
    for(int element : v) {
        if(!bst->insert(element)) return false;
    }
    return true;
}


bool TestBST::compare(vector<int> vec) {
    vector<int> bst_vec = bst->trans2vec();
    return bst_vec == vec;
}


bool TestBST::compare_set(unordered_set<int> st) {
    LockFreeBST * lf_bst = dynamic_cast<LockFreeBST*> (bst);
    if (!lf_bst) {
        cout << "Error: Invalid tree type in compare set" << endl;
        return false;
    }
    vector<LFTreeNode *> node_info = lf_bst->traverse_node_info();
    if (node_info.size() != st.size()) {
        cout << "Error: Tree size mismatch" << endl;
        return false;
    }
    for (LFTreeNode * node : node_info) {
        if (GET_INTENT_FLG(node) || GET_PROMOTE_FLG(node) || GET_DELETE_FLG(node)) {
            cout << "Error: Node flag incorrect" << endl;
            return false;
        }
        int cur_key = GET_KEY_VAL(node);
        if (st.find(cur_key) == st.end()) {  // NOT found
            cout << "Error: Incorrect value" << endl;
            return false;
        }
        st.erase(cur_key);
    }
    return true;

}


void TestBST::test_all(unsigned int thread_num, unsigned int ops_num) {
    cout << "Now running against single thread tests..." << endl;
    test_all_basic();

    cout << "Now running against multi-threads tests with " << thread_num << " threads..." << endl;
    test_all_multi(thread_num, ops_num);

    cout << "Test completed" << endl;
}


void TestBST::printResult(string test_name, bool succ) {
    cout << "Test " << test_name;
    if(succ) cout << " PASSED" << endl;
    else cout << " FAILED" << endl;
}


void TestBST::test_all_basic() {
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


bool TestBST::test_search_1() {
    if(!instantiateBST(v1)) return false;
    return bst->search(16);
}


bool TestBST::test_search_2() {
    if(!instantiateBST(v1)) return false;
    return bst->search(14);
}


bool TestBST::test_search_3() {
    if(!instantiateBST(v1)) return false;
    return !bst->search(10);
}


bool TestBST::test_insert_1() {
    if(!instantiateBST(v1)) return false;
    return compare(v1_pre_trav);
}


bool TestBST::test_insert_2() {
    if(!instantiateBST(v1)) return false;
    if(!bst->insert(10)) return false;
    return compare({16, 12, 7, 5, 11, 8, 10, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
}


bool TestBST::test_insert_3() {
    if(!instantiateBST(v1)) return false;
    return !bst->insert(36);
}


bool TestBST::test_delete_1() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(56)) return false;
    return compare({16, 12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55});
}


bool TestBST::test_delete_2() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(88)) return false;
    return compare({16, 12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 55, 56});
}


bool TestBST::test_delete_3() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(12)) return false;
    if(dynamic_cast<FineGrainedBST*> (bst)) return compare({16, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
    else return compare({16, 13, 7, 5, 11, 8, 15, 14, 33, 32, 36, 34, 88, 55, 56});
}


bool TestBST::test_delete_4() {
    if(!instantiateBST(v1)) return false;
    if(!bst->remove(16)) return false;
    if(dynamic_cast<FineGrainedBST*> (bst)) return compare({12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56});
    else return compare({32, 12, 7, 5, 11, 8, 13, 15, 14, 33, 36, 34, 88, 55, 56});
}


bool TestBST::test_combo_1() {
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


bool TestBST::test_combo_2() {
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


void TestBST::test_all_multi(unsigned int thread_num, unsigned int ops_num) {
    this->thread_num = thread_num;
    this->ops_num = ops_num;
    unsigned int total_ops = thread_num * ops_num;
    v2.resize(total_ops);
    for(int i = 0; i < total_ops; ++i) {
        v2[i] = i;
    }
    printResult("TEST_MULTI_SEARCH", test_multi_search());
    printResult("TEST_MULTI_INSERT", test_multi_insert());
    printResult("TEST_MULTI_DELETE", test_multi_delete());

    // Test with validation
    printResult("TEST_MULTI_INSERT_AND_TREE_VALIDATION", test_multi_insert_and_tree_validation());
}


bool TestBST::test_multi_search() {
    if(!instantiateBST(v2)) return false;
    bool result = true;
    vector<std::future<bool>> ret(thread_num);
    for(int tid = 0; tid < thread_num; ++tid) {
        ret[tid] = std::async(std::launch::async, test_multi_search_helper, bst, tid * ops_num, ops_num);
    }
    for(int tid = 0; tid < thread_num; ++tid) {
        result &= ret[tid].get();
    }
    return result;
}


bool TestBST::test_multi_insert() {
    if(!instantiateBST(v2)) return false;
    bool result = true;
    vector<std::future<bool>> ret(thread_num);
    for(int tid = 0; tid < thread_num; ++tid) {
        ret[tid] = std::async(std::launch::async, test_multi_insert_helper, bst, (tid + thread_num) * ops_num, ops_num);
    }
    for(int tid = 0; tid < thread_num; ++tid) {
        result &= ret[tid].get();
    }
    return result;
}


bool TestBST::test_multi_delete() {
    if(!instantiateBST(v2)) return false;
    bool result = true;
    vector<std::future<bool>> ret(thread_num);
    for(int tid = 0; tid < thread_num; ++tid) {
        ret[tid] = std::async(std::launch::async, test_multi_delete_helper, bst, tid * ops_num, ops_num);
    }
    for(int tid = 0; tid < thread_num; ++tid) {
        result &= ret[tid].get();
    }
    return result;
}


bool TestBST::test_multi_insert_and_tree_validation() {
    // Make min(thread_num, v1.size() to be the actual thread number)
    int t_num = thread_num > v1.size() ? v1.size() : thread_num;
    bool result = true;
    vector<vector<int>> vecs(t_num);
    for (int i = 0; i < v1.size(); ++i) {
        vecs[i%t_num].push_back(v1[i]);
    }

    vector<std::future<bool>> ret(t_num);
    for (int tid = 0; tid < t_num; ++tid) {
        ret[tid] = std::async(std::launch::async, test_multi_insert_helper_vector, bst, vecs[tid]);
    }
    for(int tid = 0; tid < thread_num; ++tid) {
        result &= ret[tid].get();
    }
    if (!result) return false;
    // compare set
    return compare_set(st1);
}


bool TestBST::test_multi_delete_and_tree_validation() {
    return false;
}
