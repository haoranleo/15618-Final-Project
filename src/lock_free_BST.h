//
// Created by Zoeeeee on 4/24/20.
//

#ifndef BINARY_SEARCH_TREE_LOCK_FREE_BST_H
#define BINARY_SEARCH_TREE_LOCK_FREE_BST_H

#include <limits.h>

#include "BST.h"

class LockFreeBST : public BinarySearchTree {
public:
    LockFreeBST() {
        // Construct the sentinel nodes
        root_r = new LFTreeNode(INT_MAX - 2);
        root_s = new LFTreeNode(INT_MAX - 1);
        root_t = new LFTreeNode(INT_MAX);
        root_r->right = root_s;
        root_s->right = root_t;
    }

    ~LockFreeBST() override {
        destroy(root_r);
        root_r = root_s = root_t = nullptr;
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
    SeekRecord target_record;

    /* Destroy current BST
     * @param cur : Pointer to the current tree node.
     */
    void destroy(LFTreeNode* cur);

    /* Helper function to pre-traverse the BST and write values to vector
     * @param cur : Pointer to the current tree node.
     * @param v : Vector to store the values in BST.
     */
    void trans2vec_helper(LFTreeNode* cur, vector<int>& v);

    /* Execute the seek phase required for search, insert and remove API.
     * The seek function traverses the tree from the root node until it
     * either finds the target key or reaches a non-binary node whose
     * next edge to be followed points to a null node.
     * @param target_key : Target key.
     * @param seek_record : Structure used to record the address of the
     *        terminated node, the address of its parent and the null
     *        address stored in the child field of the terminal node.
     * */
    void seek(int target_key, SeekRecord* seek_record);

};

#endif //BINARY_SEARCH_TREE_LOCK_FREE_BST_H
