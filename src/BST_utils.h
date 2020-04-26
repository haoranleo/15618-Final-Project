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


/********** Structures for node inside BST with fine-grained locking ***********/

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


/********** Structures used in lock free BST *************/

// The following bit from the address of node key is stolen by the function
// and used to indicate whether the key value is original one or replaced one
#define MODIFY_BIT 1

// The following bits from the address of node child is stolen by the function
// and are used to indicate the state of the edge between current node and child.
// The padding of the struct ensures that the lowest four bits won't be used as an address.
#define NULL_BIT 1          // Indicate whether the address field contains a null or a non-null value
#define INTENT_BIT 2        // Mark for intent
#define DELETE_BIT 4        // Mark for delete
#define PROMOTE_BIT 8       // Mark for promote

#define KEY_PTR(x) (reinterpret_cast<int*> (x))
#define NODE_PTR(x) (reinterpret_cast<LFTreeNode*> (x))
#define LNG(x) (reinterpret_cast<unsigned long> (x))
#define BOL(x) (LNG(x) != 0)

#define GET_MODIFY_FLG(x) (BOL(LNG(x) & MODIFY_BIT))
#define SET_MODIFY_FLG(x) ((x) = KEY_PTR(LNG(x) | MODIFY_BIT))
#define RESET_MODIFY_FLG(x) ((x) = KEY_PTR(LNG(x) & (~MODIFY_BIT)))
#define WITH_MODIFY_FLG(x) (KEY_PTR(LNG(x) | MODIFY_BIT))

#define GET_NULL_FLG(x) (BOL(LNG(x) & NULL_BIT))
#define GET_INTENT_FLG(x) (BOL(LNG(x) & INTENT_BIT))
#define GET_DELETE_FLG(x) (BOL(LNG(x) & DELETE_BIT))
#define GET_PROMOTE_FLG(x) (BOL(LNG(x) & PROMOTE_BIT))

#define SET_NULL_FLG(x) ((x) = NODE_PTR(LNG(x) | NULL_BIT))
#define SET_INTENT_FLG(x) ((x) = NODE_PTR(LNG(x) | INTENT_BIT))
#define SET_DELETE_FLG(x) ((x) = NODE_PTR(LNG(x) | DELETE_BIT))
#define SET_PROMOTE_FLG(x) ((x) = NODE_PTR(LNG(x) | PROMOTE_BIT))

#define RESET_NULL_FLG(x) ((x) = NODE_PTR(LNG(x) & (~NULL_BIT)))
#define RESET_INTENT_FLG(x) ((x) = NODE_PTR(LNG(x) & (~INTENT_BIT)))
#define RESET_DELETE_FLG(x) ((x) = NODE_PTR(LNG(x) & (~DELETE_BIT)))
#define RESET_PROMOTE_FLG(x) ((x) = NODE_PTR(LNG(x) & (~PROMOTE_BIT)))

#define WITH_NULL_FLG(x) (NODE_PTR(LNG(x) | NULL_BIT))
#define WITH_INTENT_FLG(x) (NODE_PTR(LNG(x) | INTENT_BIT))
#define WITH_DELETE_FLG(x) (NODE_PTR(LNG(x) | DELETE_BIT))
#define WITH_PROMOTE_FLG(x) (NODE_PTR(LNG(x) | PROMOTE_BIT))

#define GET_KEY_ADDR(x) (KEY_PTR(LNG(x) & (~MODIFY_BIT)))
#define GET_NODE_ADDR(x) (NODE_PTR(LNG(x) & (~(NULL_BIT | INTENT_BIT | DELETE_BIT | PROMOTE_BIT))))

#define GET_KEY_VAL(x) (*GET_KEY_ADDR(GET_NODE_ADDR(x)->key))
#define GET_LEFT_CHILD(x) (NODE_PTR(GET_NODE_ADDR(x)->left))
#define GET_RIGHT_CHILD(x) (NODE_PTR(GET_NODE_ADDR(x)->right))

class LFTreeNode {
public:
    LFTreeNode(): ready_to_replace(false) {
        key = new int();
        left = nullptr;
        SET_NULL_FLG(left);
        right = nullptr;
        SET_NULL_FLG(right);
    }

    explicit LFTreeNode(int v): LFTreeNode() {
        *key = v;
    }

    ~LFTreeNode() {
        delete key;
        left = nullptr; right = nullptr;
    }

//    // Structure for key in lock free BST node
//    struct LFNodeKey {
//        LFNodeKey() = default;
//        int value;  // Actual value of key
//    };
//
//    struct LFNodeChild {
//        LFNodeChild(): child(nullptr), padding(nullptr) {}
//        explicit LFNodeChild(LFTreeNode* node): child(node), padding(nullptr) {}
//        ~LFNodeChild() { child = nullptr; }     // Child should have been deleted in previous recursive destroy process
//
//        LFTreeNode *child;
//        const void* padding;      // Used to ensure that the address of any LFNodeChild won't use the lowest four bits
//    };

    // Data in tree node
    int* key;
    LFTreeNode* left;
    LFTreeNode* right;
    bool ready_to_replace;
};

// Indicate whether an edge is a left or right edge of parent node
enum EdgeType { LEFT, RIGHT, INIT };

// Structure for lock free tree edges
struct LFTreeEdge {
    LFTreeEdge(): parent(nullptr), child(nullptr), type(EdgeType::INIT) {}
    LFTreeEdge(LFTreeNode* p, LFTreeNode* c, EdgeType t): parent(p), child(c), type(t) {}
    LFTreeNode *parent;  // Parent node
    LFTreeNode *child;  // Child node
    EdgeType type;
};

// Record for seek phase
struct SeekRecord {
    SeekRecord() = default;
    LFTreeEdge last_edge;
    LFTreeEdge p_last_edge;
    LFTreeEdge inject_edge;
};

// Record for anchor node
struct AnchorRecord {
    AnchorRecord(LFTreeNode* n, int v): node(n), key_val(v) {}
    ~AnchorRecord() { node = nullptr; }
    LFTreeNode* node;
    int key_val;

    // Overloaded operator ==
    bool operator==(const AnchorRecord& a) {
        return (node == a.node && key_val == a.key_val);
    }
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
