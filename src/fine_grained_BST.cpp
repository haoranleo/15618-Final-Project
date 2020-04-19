//
// Created by Leonard on 2020/4/19.
//

#include "fine_grained_BST.h"

bool FineGrainedBST::search(int v) {
    // Must acquire the head lock before doing anything
    root_lock->lock();

    // If the BST is empty
    if(!root) {
        root_lock->unlock();
        return false;
    }

    // Search with hand-over-hand locking
    TreeNode* cur = root, *prev = nullptr;
    cur->lock->lock();
    root_lock->unlock();
    while(true) {
        if(cur->val == v) {         // Find the specific node
            cur->lock->unlock();
            return true;
        } else if (cur->val < v) {  // Continue searching in right subtree
            prev = cur;
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            prev = cur;
            cur = cur->left;
        }
        if(!cur) {                  // Subtree is empty, stop searching
            prev->lock->unlock();
            break;
        }
        cur->lock->lock();
        prev->lock->unlock();
    }
    return false;
}


bool FineGrainedBST::insert(int v) {
    // Must acquire the head lock before doing anything
    root_lock->lock();

    // If the BST is empty, initialize root node with the given value
    if(!root) {
        root = new TreeNode(v);
        root_lock->unlock();
        return true;
    }

    // Search with hand-over-hand locking
    TreeNode* cur = root, *prev = nullptr;
    cur->lock->lock();
    root_lock->unlock();
    while(true) {
        if(cur->val == v) {         // Find the node with duplicate value
            cur->lock->unlock();
            return false;
        } else if (cur->val < v) {  // Continue searching in right subtree
            if(!cur->right) {       // Right child is empty, place new node here
                cur->right = new TreeNode(v);
                cur->lock->unlock();
                break;
            }
            prev = cur;
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            if(!cur->left) {        // Left child is empty, place new node here
                cur->left = new TreeNode(v);
                cur->lock->unlock();
                break;
            }
            prev = cur;
            cur = cur->left;
        }
        cur->lock->lock();
        prev->lock->unlock();
    }
    return true;
}


bool FineGrainedBST::remove(int v) {
    // Must acquire the head lock before doing anything
    root_lock->lock();

    // If the BST is empty
    if(!root) {
        root_lock->unlock();
        return false;
    } else if(root->val == v) { // If the deleted node is root
        delete_root();
        root_lock->unlock();
        return true;
    }

    // Search with hand-over-hand locking
    TreeNode* cur = root, *prev = nullptr, *prev_par = nullptr;
    cur->lock->lock();
    root_lock->unlock();
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
                TreeNode* tmp_cur = cur->left, *tmp_prev = nullptr;
                tmp_cur->lock->lock();
                while(true) {
                    tmp_prev = tmp_cur;
                    if(tmp_cur->right) tmp_cur = tmp_cur->right;
                    else break;
                    tmp_cur->lock->lock();
                    tmp_prev->lock->unlock();
                }
                tmp_cur->right = cur->right;
                tmp_cur->lock->unlock();
                if(prev->val > cur->val) prev->left = cur->left;
                else prev->right = cur->left;
            }
            prev->lock->unlock();
            cur->lock->unlock();
            delete cur;
            return true;
        } else if (cur->val < v) {  // Continue searching in right subtree
            prev_par = prev;
            prev = cur;
            cur = cur->right;
        } else {                    // Continue searching in left subtree
            prev_par = prev;
            prev = cur;
            cur = cur->left;
        }
        if(!cur) {                  // Subtree is empty, stop searching
            if(prev_par) prev_par->lock->unlock();
            prev->lock->unlock();
            break;
        }
        cur->lock->lock();
        if(prev_par) prev_par->lock->unlock();
    }
    return false;
}


void FineGrainedBST::delete_root() {
    TreeNode* cur = root;
    cur->lock->lock();

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
        TreeNode* tmp_cur = cur->left, *tmp_prev = nullptr;
        tmp_cur->lock->lock();
        while(true) {
            tmp_prev = tmp_cur;
            if(tmp_cur->right) tmp_cur = tmp_cur->right;
            else break;
            tmp_cur->lock->lock();
            tmp_prev->lock->unlock();
        }
        tmp_cur->right = cur->right;
        tmp_cur->lock->unlock();
        root = cur->left;
    }
    cur->lock->unlock();
    delete cur;
}
