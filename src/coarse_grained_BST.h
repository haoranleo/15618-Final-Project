//
// Created by Leonard on 2020/5/1.
//

#ifndef INC_15618_FINE_GRAINED_BST_COARSE_GRAINED_BST_H
#define INC_15618_FINE_GRAINED_BST_COARSE_GRAINED_BST_H

#include "BST.h"
#include <mutex>

class CoarseGrainedBST : public BinarySearchTree {
public:
    CoarseGrainedBST(): root(nullptr) {}

    ~CoarseGrainedBST() override {
        destroy(root);
        root = nullptr;
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

    /* Print detailed information about nodes in the tree */
    void print_info() override;

private:
    Node* root;
    std::mutex lock;

    /* Helper function used to delete root node of the BST. */
    void delete_root();

    /* Destroy current BST
     * @param cur : Pointer to the current tree node.
     */
    void destroy(Node* cur);

    /* Helper function to pre-traverse the BST and write values to vector
     * @param cur : Pointer to the current tree node.
     * @param v : Vector to store the values in BST.
     */
    void trans2vec_helper(Node* cur, vector<int>& v);
};

#endif //INC_15618_FINE_GRAINED_BST_COARSE_GRAINED_BST_H
