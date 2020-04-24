//
// Created by Leonard on 2020/4/23.
//

#include "benchmark.h"

bool Benchmark::initBST() {
    vector<int> v;
    for(int i = 0; i < init_tree_size; ++i) {
        v.emplace_back(init_gap * i);
    }
    shuffle(v.begin(), v.end(), std::default_random_engine(0)); // Shuffle in pseudo-random style
    return instantiateBST(v);
}


bool Benchmark::instantiateBST(vector<int> v) {
    bst->reinitialize();
    for(int element : v) {
        if(!bst->insert(element)) return false;
    }
    return true;
}


void Benchmark::test_all_read(float content_factor) {
    // TODO: add timer

    // Collection of worker thread
    vector<thread> threads;

    // Random number generator
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, init_tree_size - 1);
    auto dice = std::bind(distribution, generator);

    // Spawn threads to execute read workload
    for(int tid = 0; tid < thread_num; ++tid) {
        vector<int> test_numbers;
        for(int i = 0; i < ops_num; ++i) {
            test_numbers.emplace_back(dice() * init_gap);
        }
        thread t(thread_search, bst, test_numbers);
        threads.emplace_back(t);
    }

    for(int tid = 0; tid < thread_num; ++tid) threads[tid].join();
}