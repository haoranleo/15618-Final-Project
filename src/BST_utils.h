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


/********** Structures for lock free BST *************/
class LFTreeNode {
public:
    LFTreeNode(): ready_to_replace(false) {
        left = new LFNodeChild();
        right = new LFNodeChild();
    }
    ~LFTreeNode() {
        delete left; delete right;
    }

    // Structure for key in lock free BST node
    struct LFNodeKey {
        LFNodeKey(): modify_flg(false), value(0) {}
        bool modify_flg;  // Indicate whether the value is original one or replaced one
        int value;  // Actual value of key
    };

    struct LFNodeChild {
        LFNodeChild(): intent_flg(false), delete_flg(false),
                       promote_flg(false), null_flg(false), child(nullptr) {}
        ~LFNodeChild() { child = nullptr; }     // Child should have been deleted in previous recursive destroy process

        // Flags indicate state of the edge between current node and child
        bool intent_flg;  // mark for intent
        bool delete_flg;  // mark for delete
        bool promote_flg;  // mark for promote

        bool null_flg;  // indicate whether the address field contains a null or a non-null value

        LFTreeNode *child;
    };

    // Data in tree node
    LFNodeKey key;
    LFNodeChild* left;
    LFNodeChild* right;
    bool ready_to_replace;
};

// Indicate whether an edge is a left or right edge of parent node
enum EdgeType { LEFT, RIGHT };

// Structure for lock free tree edges
struct LFTreeEdge {
    LFTreeNode *parent;  // Parent node
    LFTreeNode *child;  // Child node
    EdgeType type;
};

// Record for seek phase
struct SeekRecord {
    LFTreeEdge last_edge;
    LFTreeEdge p_last_edge;
    LFTreeEdge inject_edge;
};

// Record for anchor node
struct AnchorRecord {
    LFTreeNode* node;
    int key_val;
};

enum DelMode { INJECT, DISCOVERY, CLEANUP };
enum DelType { SIMPLE, COMPLEX };

// Record of current execution state
struct StateRecord {
    LFTreeEdge target_edge;
    LFTreeEdge p_target_edge;

    int target_key;
    int current_key;

    DelMode mode;
    DelType type;

    // the next field stores pointer to a seek record; it is used
    // for finding the successor if the delete operation is complex
    SeekRecord *successorRecord;
};


#endif //INC_15618_FINE_GRAINED_BST_UTILS_H
