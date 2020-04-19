//
// Created by Leonard on 2020/4/19.
//

#ifndef INC_15618_FINE_GRAINED_BST_BST_H
#define INC_15618_FINE_GRAINED_BST_BST_H

#include "BST_utils.h"

// Structure of BST
class BinarySearchTree {
public:
    BinarySearchTree(): root(nullptr) {};
    ~BinarySearchTree() {
        delete root;
    };
    // Determine whether the given value exists in the tree.
    virtual bool search(int v) = 0;
    // Insert new value into the tree.
    virtual bool insert(int v) = 0;
    // Remove value from the tree.
    virtual bool remove(int v) = 0;
protected:
    TreeNode* root;
};

#endif //INC_15618_FINE_GRAINED_BST_BST_H
