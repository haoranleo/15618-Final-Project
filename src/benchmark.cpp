//
// Created by Leonard on 2020/4/23.
//

#include "benchmark.h"

void Benchmark::run_coarse_grained_BST(unsigned int size, unsigned int thread_num) {
    cout << "=========Run experiments using coarse grained BST========" << endl;
    init_tree_size = size;
    this->thread_num = thread_num;
    bst = new CoarseGrainedBST();
    init_BST_elements();
    run_benchmark();
}


void Benchmark::run_fine_grained_BST(unsigned int size, unsigned int thread_num) {
    cout << "=========Run experiments using fine grained BST========" << endl;
    init_tree_size = size;
    this->thread_num = thread_num;
    bst = new FineGrainedBST();
    init_BST_elements();
    run_benchmark();
}


void Benchmark::run_lock_free_BST(unsigned int size, unsigned int thread_num) {
    cout << "=========Run experiments using lock free BST========" << endl;
    init_tree_size = size;
    this->thread_num = thread_num;
    bst = new LockFreeBST();
    init_BST_elements();
    run_benchmark();
}


void Benchmark::run_benchmark() {
    cout << "****** Read-only experiments ******" << endl;
    // Read-only experiments
    float content_factor_gap = 0.2;
    for(int eid = 0; eid <= 5; ++eid) {
        float content_factor = eid * content_factor_gap;
        if(!instantiate_BST(tree_elements)) return;
        test_all_read(content_factor);
    }

    cout << "****** Insert-only experiments ******" << endl;
    // Insert-only experiments
    for(int eid = 0; eid <= 5; ++eid) {
        float content_factor = eid * content_factor_gap;
        if(!instantiate_BST({})) return;
        test_all_insert(content_factor);
    }

    cout << "****** Delete-only experiments ******" << endl;
    // Delete-only experiments
    for(int eid = 0; eid <= 5; ++eid) {
        float content_factor = eid * content_factor_gap;
        if(!instantiate_BST(tree_elements)) return;
        test_all_delete(content_factor);
    }

    cout << "****** Random instruction experiments ******" << endl;
    // Random operations experiments
    for(int eid = 0; eid <= 5; ++eid) {
        float content_factor = eid * content_factor_gap;
        if(!instantiate_BST(tree_elements)) return;
        test_all_random(content_factor);
    }
}

void Benchmark::init_BST_elements() {
    if(!tree_elements.empty()) return;

//    for(int i = 0; i < init_tree_size; ++i) tree_elements.emplace_back(i);
//    shuffle(tree_elements.begin(), tree_elements.end(), std::default_random_engine(0)); // Shuffle in pseudo-random style

    generate_balanced_tree_elements(tree_elements, 0, init_tree_size);

//    for(int i : tree_elements) cout << i << " ";
//    cout << endl;
}


void Benchmark::generate_balanced_tree_elements(vector<int> &v, int start, int end) {
    if(start >= end) return;
    else if(start == end - 1) v.emplace_back(start);
    else {
        int mid = start + (end - start) / 2;
        v.emplace_back(mid);
        generate_balanced_tree_elements(v, start, mid);
        generate_balanced_tree_elements(v, mid + 1, end);
    }
}


bool Benchmark::instantiate_BST(vector<int> v) {
    bst->reinitialize();
    for(int element : v) {
        if(!bst->insert(element)) return false;
    }
    return true;
}


void Benchmark::assign_test_numbers(vector<int> &test_numbers, int tid, float content_factor) {
    int num_per_thread = init_tree_size / thread_num;
    int chunk_size = num_per_thread - int((num_per_thread - 1) * content_factor);

    for(int cid = 0; cid * chunk_size < init_tree_size; ++cid) {
        int start = cid * chunk_size * thread_num + chunk_size * tid;
        int end = start + chunk_size;
        for(int i = start; i < end; ++i) {
            test_numbers.emplace_back(i);
            if(test_numbers.size() >= num_per_thread) {
                shuffle(test_numbers.begin(), test_numbers.end(), engine);
                return;
            }
        }
    }
}


void Benchmark::test_all_read(float content_factor) {
    // Collection of worker thread
    vector<thread> threads(thread_num);

    // Assign target numbers for different thread
    vector<vector<int>> test_numbers(thread_num, vector<int>());
    for(int tid = 0; tid < thread_num; ++tid) {
        assign_test_numbers(test_numbers[tid], tid, content_factor);
    }

    double start_time = currentSeconds();

    // Spawn threads to execute read workload
    for(int tid = 0; tid < thread_num; ++tid) {
        threads[tid] = thread(thread_search, bst, test_numbers[tid]);
//        threads.emplace_back(t);
    }
    for(int tid = 0; tid < thread_num; ++tid) threads[tid].join();

    // Record elapsed time
    double end_time = currentSeconds();
    cout << "Content factor: " << content_factor << " | Elapsed time: " << (end_time - start_time) * 1000 << " ms" << endl;
}


void Benchmark::test_all_insert(float content_factor) {
    // Collection of worker thread
    vector<thread> threads(thread_num);

    // Assign target numbers for different thread
    vector<vector<int>> test_numbers(thread_num, vector<int>());
    for(int tid = 0; tid < thread_num; ++tid) {
        assign_test_numbers(test_numbers[tid], tid, content_factor);
    }

    double start_time = currentSeconds();

    // Spawn threads to execute read workload
    for(int tid = 0; tid < thread_num; ++tid) {
        threads[tid] = thread(thread_insert, bst, test_numbers[tid]);
//        threads.emplace_back(t);
    }
    for(int tid = 0; tid < thread_num; ++tid) threads[tid].join();

    // Record elapsed time
    double end_time = currentSeconds();
    cout << "Content factor: " << content_factor << " | Elapsed time: " << (end_time - start_time) * 1000 << " ms" << endl;
}


void Benchmark::test_all_delete(float content_factor) {
    // Collection of worker thread
    vector<thread> threads(thread_num);

    // Assign target numbers for different thread
    vector<vector<int>> test_numbers(thread_num, vector<int>());
    for(int tid = 0; tid < thread_num; ++tid) {
        assign_test_numbers(test_numbers[tid], tid, content_factor);
    }

    double start_time = currentSeconds();

    // Spawn threads to execute read workload
    for(int tid = 0; tid < thread_num; ++tid) {
        threads[tid] =  thread(thread_delete, bst, test_numbers[tid]);
//        threads.emplace_back(t);
    }
    for(int tid = 0; tid < thread_num; ++tid) threads[tid].join();

    // Record elapsed time
    double end_time = currentSeconds();
    cout << "Content factor: " << content_factor << " | Elapsed time: " << (end_time - start_time) * 1000 << " ms" << endl;
}


void Benchmark::test_all_random(float content_factor) {
    // Collection of worker thread
    vector<thread> threads(thread_num);

    // Assign target numbers for different thread
    vector<vector<int>> test_numbers(thread_num, vector<int>());
    for(int tid = 0; tid < thread_num; ++tid) {
        assign_test_numbers(test_numbers[tid], tid, content_factor);
    }

    double start_time = currentSeconds();

    // Spawn threads to execute read workload
    for(int tid = 0; tid < thread_num; ++tid) {
        threads[tid] = thread(thread_random, bst, test_numbers[tid], tid);
//        threads.emplace_back(t);
    }
    for(int tid = 0; tid < thread_num; ++tid) threads[tid].join();

    // Record elapsed time
    double end_time = currentSeconds();
    cout << "Content factor: " << content_factor << " | Elapsed time: " << (end_time - start_time) * 1000 << " ms" << endl;
}