//
// Created by Zoeeeee on 4/24/20.
//

#ifndef BINARY_SEARCH_TREE_LOCK_FREE_BST_H
#define BINARY_SEARCH_TREE_LOCK_FREE_BST_H

#include "BST.h"
#include "BST_utils.h"

class LockFreeBST : public BinarySearchTree {
public:
    LockFreeBST() {
        // Construct the sentinel nodes
        root_r = new LFTreeNode();
        root_s = new LFTreeNode();
        root_t = new LFTreeNode();
        root_r->right->child = root_s;
        root_s->right->child = root_t;
    }

    ~LockFreeBST() override {
        // TODO: add code to delete all nodes
    }

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

    /* Destroy current BST and reconstruct an empty one. */
    void reinitialize() override;

    /* Write all nodes' value to vector in pre-traversal order
     * @return : Vector containing all values in the BST in pre-traversal order
     */
    vector<int> trans2vec() override;

private:
    LFTreeNode *root_r, *root_s, *root_t; // They are all sentinel nodes

    /* Destroy current BST
     * @param cur : Pointer to the current tree node.
     */
    void destroy(LFTreeNode* cur);

    /* Helper function to pre-traverse the BST and write values to vector
     * @param cur : Pointer to the current tree node.
     * @param v : Vector to store the values in BST.
     */
    void trans2vec_helper(LFTreeNode* cur, vector<int>& v);
};

#endif //BINARY_SEARCH_TREE_LOCK_FREE_BST_H
