//
// Created by Zoeeeee on 4/24/20.
//

#ifndef BINARY_SEARCH_TREE_LOCK_FREE_BST_H
#define BINARY_SEARCH_TREE_LOCK_FREE_BST_H

#include "BST.h"

class LockFreeBST : public BinarySearchTree {
public:
    LockFreeBST() = default;

    ~LockFreeBST() = default;


    /* Traverse the BST to determine whether the given value exists.
     * @param v : Target value to find.
     * @return: true if the value is found. Otherwise, return false.
     */
    bool search(int v) override;

    /* Insert a new node with specified value into BST.
     * @param v : Value of the node to be inserted.
     * @return: true if successfully inserts the node.
     *          false if there has already existed a node with the same value.
     */
    bool insert(int v) override;

    /* Delete a new node with specified value from BST.
     * @param v : Value of the node to be deleted.
     * @return: true if successfully deletes the node.
     *          false if the node doesn't exist or the tree is empty.
     */
    bool remove(int v) override;
};

#endif //BINARY_SEARCH_TREE_LOCK_FREE_BST_H
