//
// Created by Leonard on 2020/4/23.
//

#include "BST.h"

void BinarySearchTree::reinitialize() {
    destroy(root);
    root = nullptr;
}


void BinarySearchTree::destroy(TreeNode *cur) {
    if(!cur) return;
    destroy(cur->left);
    destroy(cur->right);
    delete cur;
}


vector<int> BinarySearchTree::trans2vec() {
    vector<int> res;
    trans2vec_helper(root, res);
    return res;
}


void BinarySearchTree::trans2vec_helper(TreeNode *cur, vector<int> &v) {
    if(!cur) return;
    v.push_back(cur->val);
    trans2vec_helper(cur->left, v);
    trans2vec_helper(cur->right, v);
}

