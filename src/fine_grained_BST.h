//
// Created by Leonard on 2020/4/19.
//

#ifndef INC_15618_FINE_GRAINED_BST_FINE_GRAINED_BST_H
#define INC_15618_FINE_GRAINED_BST_FINE_GRAINED_BST_H

#include "BST.h"

class FineGrainedBST : public BinarySearchTree {
    FineGrainedBST() {
        root_lock = new Spinlock();
    }

    ~FineGrainedBST() {
        delete root_lock;
    }

    /* Traverse the BST to determine whether the given value exists.
     * @param v : Target value to find.
     * @return: true if the value is found. Otherwise, return false.
     */
    bool search(int v);

    /* Insert a new node with specified value into BST.
     * @param v : Value of the node to be inserted.
     * @return: true if successfully inserts the node.
     *          false if there has already existed a node with the same value.
     */
    bool insert(int v);

    /* Delete a new node with specified value from BST.
     * @param v : Value of the node to be deleted.
     * @return: true if successfully deletes the node.
     *          false if the node doesn't exist or the tree is empty.
     */
    bool remove(int v);

private:
    /* Helper function used to delete root node of the BST.
     */
    void delete_root();
    Spinlock* root_lock;    // Lock must be acquired before doing any operation
};
#endif //INC_15618_FINE_GRAINED_BST_FINE_GRAINED_BST_H
