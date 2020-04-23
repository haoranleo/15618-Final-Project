//
// Created by Leonard on 2020/4/19.
//

#ifndef INC_15618_FINE_GRAINED_BST_UTILS_H
#define INC_15618_FINE_GRAINED_BST_UTILS_H

#include <atomic>
#include <iostream>
#include <string>
#include <vector>

using std::vector;
using std::string;
using std::cout;
using std::endl;

// Spin lock
class Spinlock {
public:
    // Attempt to acquire lock in spinning style
    void lock() {
        while (lock_flg.test_and_set(std::memory_order_acquire));
    }

    // Release the lock by simply setting flag to false
    void unlock() {
        lock_flg.clear(std::memory_order_release);
    }
private:
    std::atomic_flag lock_flg = ATOMIC_FLAG_INIT;
};


// Structure of the basic node inside BST
class TreeNode {
public:
    TreeNode(int v): val(v), left(nullptr), right(nullptr) {
        lock = new Spinlock();
    }
    ~TreeNode() {
        left = nullptr; right = nullptr;
        delete lock;
    }
    int val;            // Node value
    TreeNode* left;     // Left child
    TreeNode* right;    // Right child
    Spinlock* lock;     // Sping lock
};


#endif //INC_15618_FINE_GRAINED_BST_UTILS_H
