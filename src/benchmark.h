//
// Created by Leonard on 2020/4/23.
//

#ifndef INC_15618_FINE_GRAINED_BST_BENCHMARK_H
#define INC_15618_FINE_GRAINED_BST_BENCHMARK_H

#include <algorithm>
#include <functional>
#include <random>
#include <thread>

#include "fine_grained_BST.h"

using std::thread;

class Benchmark {
    Benchmark(int size): init_tree_size(size), bst(nullptr) {}
    ~Benchmark() {
        if(!bst) return;
        FineGrainedBST* fg_bst = dynamic_cast<FineGrainedBST*> (bst);
        if(fg_bst) delete fg_bst;
        else {
            // TODO: add lock-free bst obj cast here
        }
    }
    // TODO: test API, should accept tree size, thread_num, content factor (?)
private:
    BinarySearchTree *bst;
    int init_tree_size = 100;
    int init_gap = 10;
    int thread_num = 4;
    int ops_num = 10;

    /* Initialize and propagate underlying BST to specified size */
    bool initBST();

    /* Construct new BST from scratch by inserting element in
     * the input vector in order.
     * @param v : The input vector containing elements to be inserted.
     * @return: true if successfully inserts all values.
     *          false if the vector contains duplicates.
     */
    bool instantiateBST(vector<int> v);

    // TODO: read / write pattern
    /* Read-only experiment routine. Spawn specified number of threads
     * and run read-only workload concurrently. Measure the elapsed time
     * and display to stdout.
     * @ param content_factor : Identify the degree of contention.
     */
    void test_all_read(float content_factor);

    /* Thread routine. Single thread completes multiple search operations.
     * @param bst : Pointer to underlying binary search tree object.
     * @param test_numbers : Vector containing numbers to be searched.
     */
    static void thread_search(BinarySearchTree* bst, vector<int> test_numbers) {
        for(int num : test_numbers) {
            bst->search(num);
        }
    }

    /* Thread routine. Single thread completes multiple insert operations.
     * @param bst : Pointer to underlying binary search tree object.
     * @param test_numbers : Vector containing numbers to be inserted.
     */
    static void thread_insert(BinarySearchTree* bst, vector<int> test_numbers) {

    }

    /* Thread routine. Single thread completes multiple delete operations.
     * @param bst : Pointer to underlying binary search tree object.
     * @param test_numbers : Vector containing numbers to be deleted.
     */
    static void thread_delete(BinarySearchTree* bst, vector<int> test_numbers) {

    }
};

#endif //INC_15618_FINE_GRAINED_BST_BENCHMARK_H
