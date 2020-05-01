//
// Created by Leonard on 2020/5/1.
//

#include "coarse_grained_BST.h"

bool CoarseGrainedBST::search(int v) {
    std::lock_guard<std::mutex> lock_guard(lock);
    if(!root) return false;

    Node* cur = root;
    while(true) {
        if(cur->val == v) {         // Find the specific node
            return true;
        } else if (cur->val < v) {  // Continue searching in right subtree
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            cur = cur->left;
        }
        if(!cur) {                  // Subtree is empty, stop searching
            break;
        }
    }
    return false;
}


bool CoarseGrainedBST::insert(int v) {
    std::lock_guard<std::mutex> lock_guard(lock);
    if(!root) {
        root = new Node(v);
        return true;
    }

    Node* cur = root;
    while(true) {
        if(cur->val == v) {         // Find the node with duplicate value
            return false;
        } else if (cur->val < v) {  // Continue searching in right subtree
            if(!cur->right) {       // Right child is empty, place new node here
                cur->right = new Node(v);
                break;
            }
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            if(!cur->left) {        // Left child is empty, place new node here
                cur->left = new Node(v);
                break;
            }
            cur = cur->left;
        }
    }
    return true;
}


bool CoarseGrainedBST::remove(int v) {
    std::lock_guard<std::mutex> lock_guard(lock);

    // If the BST is empty
    if(!root) {
        return false;
    } else if(root->val == v) { // If the deleted node is root
        delete_root();
        return true;
    }

    // Search with hand-over-hand locking
    Node* cur = root, *prev = nullptr;
    while(true) {
        if(cur->val == v) {         // Find the specific node
            // Case 1: deleted node doesn't have any child
            if(!cur->left && !cur->right) {
                if(prev->val > cur->val) prev->left = nullptr;
                else prev->right = nullptr;
            }

            // Case 2: deleted node has only one child
            else if (!cur->left || !cur->right) {
                if(prev->val > cur->val) {
                    if(cur->left) prev->left = cur->left;
                    else prev->left = cur->right;
                } else {
                    if(cur->left) prev->right = cur->left;
                    else prev->right = cur->right;
                }
            }

            // Case 3: deleted node has both left and right child
            else {
                // Find the largest node in left subtree
                Node* tmp_cur = cur->left;
                while(true) {
                    if(tmp_cur->right) tmp_cur = tmp_cur->right;
                    else break;
                }
                tmp_cur->right = cur->right;
                if(prev->val > cur->val) prev->left = cur->left;
                else prev->right = cur->left;
            }
            delete cur;
            return true;
        } else if (cur->val < v) {  // Continue searching in right subtree
            prev = cur;
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            prev = cur;
            cur = cur->left;
        }
        if(!cur) {                  // Subtree is empty, stop searching
            break;
        }
    }
    return false;
}


void CoarseGrainedBST::delete_root() {
    Node* cur = root;

    // Case 1: deleted node doesn't have any child
    if(!cur->left && !cur->right) {
        root = nullptr;
    }

    // Case 2: deleted node has only one child
    else if (!cur->left || !cur->right) {
        if(cur->left) root = cur->left;
        else root = cur->right;
    }

    // Case 3: deleted node has both left and right child
    else {
        // Find the largest node in left subtree
        Node* tmp_cur = cur->left, *tmp_prev = nullptr;
        while(true) {
            tmp_prev = tmp_cur;
            if(tmp_cur->right) tmp_cur = tmp_cur->right;
            else break;
        }
        tmp_cur->right = cur->right;
        root = cur->left;
    }
    delete cur;
}


void CoarseGrainedBST::reinitialize() {
    destroy(root);
    root = nullptr;
}


void CoarseGrainedBST::destroy(Node *cur) {
    if(!cur) return;
    destroy(cur->left);
    destroy(cur->right);
    delete cur;
}


vector<int> CoarseGrainedBST::trans2vec() {
    vector<int> res;
    trans2vec_helper(root, res);
    return res;
}


void CoarseGrainedBST::trans2vec_helper(Node *cur, vector<int> &v) {
    if(!cur) return;
    v.push_back(cur->val);
    trans2vec_helper(cur->left, v);
    trans2vec_helper(cur->right, v);
}


void CoarseGrainedBST::print_info() {}