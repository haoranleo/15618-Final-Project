//
// Created by Leonard on 2020/4/23.
//

#ifndef INC_15618_FINE_GRAINED_BST_TEST_FINE_GRAINED_BST_H
#define INC_15618_FINE_GRAINED_BST_TEST_FINE_GRAINED_BST_H

#include "fine_grained_BST.h"

class TestFineGrainedBST {
public:
    TestFineGrainedBST() {
        bst = new FineGrainedBST();
        v1 = {16, 12, 13, 7, 33, 36, 11, 88, 5, 32, 8, 15, 34, 14, 55, 56};
        v1_pre_trav = {16, 12, 7, 5, 11, 8, 13, 15, 14, 33, 32, 36, 34, 88, 55, 56};
    }

    ~TestFineGrainedBST() {
        delete bst;
    }

    /* Run all basic tests to check the correctness for all the
     * operations of fine grained BST.
     */
    void test_all_basic();
private:
    FineGrainedBST *bst;

    /* Construct new BST from scratch by inserting element in
     * the input vector in order.
     * @param v : The input vector containing elements to be inserted.
     * @return: true if successfully inserts all values.
     *          false if the vector contains duplicates.
     */
    bool instantiateBST(vector<int> v);

    /* Check whether the values in BST are consistent with the
     * input vector. The values in BST will be transformed into
     * vector in pre-traversal order first before comparison.
     * @param vec : Input vector to be compared with.
     * @return : true if the values in tree are consistent with the input vector.
     *           false if fails to match.
     */
    bool compare(vector<int> vec);

    /* Test data */
    vector<int> v1;
    vector<int> v1_pre_trav;

    /* Basic test function */
    bool test_search_1();
    bool test_search_2();
    bool test_search_3();
    bool test_insert_1();
    bool test_insert_2();
    bool test_insert_3();
    bool test_delete_1();
    bool test_delete_2();
    bool test_delete_3();
    bool test_delete_4();
    bool test_combo_1();
    bool test_combo_2();

    /* Helper function used in test */
    void printResult(string test_name, bool succ);
};

#endif //INC_15618_FINE_GRAINED_BST_TEST_FINE_GRAINED_BST_H
