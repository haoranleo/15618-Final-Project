//
// Created by Leonard on 2020/4/19.
//

#ifndef INC_15618_FINE_GRAINED_BST_BST_H
#define INC_15618_FINE_GRAINED_BST_BST_H

#include "BST_utils.h"

// Structure of BST
class BinarySearchTree {
public:
    virtual ~BinarySearchTree() = default;

    // Determine whether the given value exists in the tree.
    virtual bool search(int v) = 0;
    // Insert new value into the tree.
    virtual bool insert(int v) = 0;
    // Remove value from the tree.
    virtual bool remove(int v) = 0;

    // Destroy current BST and reconstruct an empty one.
    virtual void reinitialize() = 0;
    // Write all nodes' value to vector in pre-traversal order
    virtual vector<int> trans2vec() = 0;
};

#endif //INC_15618_FINE_GRAINED_BST_BST_H
