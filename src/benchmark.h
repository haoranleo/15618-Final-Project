//
// Created by Leonard on 2020/4/23.
//

#ifndef INC_15618_FINE_GRAINED_BST_BENCHMARK_H
#define INC_15618_FINE_GRAINED_BST_BENCHMARK_H

#include <algorithm>
#include <functional>
#include <random>
#include <thread>

#include "cycletimer.h"
#include "coarse_grained_BST.h"
#include "fine_grained_BST.h"
#include "lock_free_BST.h"

using std::thread;

class Benchmark {
public:
    explicit Benchmark(int size): init_tree_size(size), bst(nullptr) {}
    ~Benchmark() {
        if(!bst) return;
        delete bst;
    }

    /* Entry function to run all the experiments in benchmark using coarse grained BST.
     * @param size : Initial size of the tree.
     * @param thread_num : Number of threads to run.
     */
    void run_coarse_grained_BST(unsigned int size, unsigned int thread_num);

    /* Entry function to run all the experiments in benchmark using fine grained BST.
     * @param size : Initial size of the tree.
     * @param thread_num : Number of threads to run.
     */
    void run_fine_grained_BST(unsigned int size, unsigned int thread_num);

    /* Entry function to run all the experiments in benchmark using lock free BST.
     * @param size : Initial size of the tree.
     * @param thread_num : Number of threads to run.
     */
    void run_lock_free_BST(unsigned int size, unsigned int thread_num);

private:
    BinarySearchTree *bst;
    vector<int> tree_elements;
    unsigned int init_tree_size = 100;
    int init_gap = 10;
    unsigned int thread_num = 4;
    int ops_num = 10;
    std::default_random_engine engine;

    /* Initialize and propagate underlying BST to specified size and
     * store all elements into a vector to be used in future experiments.
     */
    void init_BST_elements();

    /* Generate balanced BST and store all elements in vector in pre-traversal order.
     * @param v : Vector containing elements in tree.
     * @param start : Left limit of current range.
     * @param end : right limit of current range.
     */
    void generate_balanced_tree_elements(vector<int>& v, int start, int end);

    /* Construct new BST from scratch by inserting element in
     * the input vector in order.
     * @param v : The input vector containing elements to be inserted.
     * @return: true if successfully inserts all values.
     *          false if the vector contains duplicates.
     */
    bool instantiate_BST(vector<int> v);

    /* Main routine to run all experiments using the specified underlying BST */
    void run_benchmark();

    /* Assign numbers for thread to operate on based on the contention factor.
     * The larger the contention factor is, the more aggressively the target
     * numbers will distribute in the possible range. If the contention factor is
     * small, the target number of different thread will intend to group into
     * different cluster and won't interleave with target numbers of other threads.
     * @param test_numbers : Vector to be populated with target numbers.
     * @param tid : Thread id.
     * @param content_factor: Used to represent the degree of contention and in range [0, 1].
     */
    void assign_test_numbers(vector<int>& test_numbers, int tid, float content_factor);

    /* Read-only experiment routine. Spawn specified number of threads
     * and run read-only workload concurrently. Measure the elapsed time
     * and display to stdout.
     * @param content_factor : Identify the degree of contention.
     */
    void test_all_read(float content_factor);

    /* Insert-only experiment routine. Spawn specified number of threads
     * and run insert-only workload concurrently. Measure the elapsed time
     * and display to stdout.
     * @param content_factor : Identify the degree of contention.
     */
    void test_all_insert(float content_factor);

    /* Delete-only experiment routine. Spawn specified number of threads
     * and run delete-only workload concurrently. Measure the elapsed time
     * and display to stdout.
     * @param content_factor : Identify the degree of contention.
     */
    void test_all_delete(float content_factor);

    /* Experiment routine to execute instructions randomly. Spawn specified
     * number of threads and run random-instruction workload concurrently.
     * Measure the elapsed time and display to stdout.
     * @param content_factor : Identify the degree of contention.
     */
    void test_all_random(float content_factor);

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
        for(int num : test_numbers) {
            bst->insert(num);
        }
    }

    /* Thread routine. Single thread completes multiple delete operations.
     * @param bst : Pointer to underlying binary search tree object.
     * @param test_numbers : Vector containing numbers to be deleted.
     */
    static void thread_delete(BinarySearchTree* bst, vector<int> test_numbers) {
        for(int num : test_numbers) {
            bst->remove(num);
        }
    }

    /* Thread routine. Single thread completes multiple operations randomly.
     * @param bst : Pointer to underlying binary search tree object.
     * @param test_numbers : Vector containing numbers to be used.
     * @param tid : Thread id.
     */
    static void thread_random(BinarySearchTree* bst, vector<int> test_numbers, unsigned int tid) {
        // Random number generator
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, 2);
        auto dice = std::bind(distribution, generator);

        for(int num : test_numbers) {
            int cur_dice = dice();
            switch(cur_dice) {
                case 0: bst->search(num); break;
                case 1: bst->insert(num); break;
                case 2: bst->remove(num); break;
                default: {
                    cout << "Dice out of possible range" << endl;
                    return;
                }
            }
        }
    }
};

#endif //INC_15618_FINE_GRAINED_BST_BENCHMARK_H
