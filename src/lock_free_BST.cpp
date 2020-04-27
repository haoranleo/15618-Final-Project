//
// Created by Zoeeeee on 4/24/20.
//

#include "lock_free_BST.h"

/* Will need to create record wherever needed */

// object to store information about the tree traversal when looking for a given key (used by the seek function)
// SeekRecordPtr targetRecord := new seek record;
// object to store information about process’ own delete operation
// StateRecordPtr myState := new state;

bool LockFreeBST::search(int v) {
    SeekRecord seek_record;
    seek(v, &seek_record);
    if(GET_KEY_VAL(seek_record.last_edge.child) == v) return true;
    return false;
}


bool LockFreeBST::insert(int v) {
    seek(v, &target_record);
    LFTreeEdge target_edge = target_record.last_edge;
    LFTreeNode* node = target_edge.child;
    if(GET_KEY_VAL(node) == v) return false;  // Exists duplicate node

    // Create new node
    EdgeType which_edge = target_record.inject_edge.type;
    LFTreeNode* old_child = target_record.inject_edge.child;
    LFTreeNode* new_child = new LFTreeNode(v);
    RESET_NULL_FLG(new_child);
    LFTreeNode** target_addr = which_edge == EdgeType::LEFT ? &(GET_NODE_ADDR(node)->left) : &(GET_NODE_ADDR(node)->right);

//    cout << *target_addr << " " << old_child << " " << new_child << endl;
    bool result = __sync_bool_compare_and_swap(target_addr, old_child, new_child);
    if(result) return true;

    // TODO: helper function
    return false;
}


bool LockFreeBST::remove(int v) {
    return false;
}


void LockFreeBST::seek(int target_key, SeekRecord *seek_record) {
    AnchorRecord prev_anchor_record(root_s, GET_KEY_VAL(root_s));
    SeekRecord p_seek_record;
    while(true) {
        // Initialize all variables used in traversal
        LFTreeEdge p_last_edge(root_r, root_s, EdgeType::RIGHT);
        LFTreeEdge last_edge(root_s, root_t, EdgeType::RIGHT);
        LFTreeNode *cur = root_t;
        AnchorRecord anchor_record(root_s, GET_KEY_VAL(root_s));

        while(true) {
            int cur_key = GET_KEY_VAL(cur);
            EdgeType which_edge = target_key > cur_key ? EdgeType::RIGHT : EdgeType::LEFT;
            LFTreeNode* raw_child = which_edge == EdgeType::LEFT ? GET_LEFT_CHILD(cur) : GET_RIGHT_CHILD(cur);
            LFTreeNode *next = raw_child;

            // If either key found or no next edge to follow, stop the traversal
            if(cur_key == target_key || GET_NULL_FLG(raw_child)) {
                seek_record->p_last_edge = p_last_edge;
                seek_record->last_edge = last_edge;
                seek_record->inject_edge = LFTreeEdge(cur, next, which_edge);
                if(cur_key == target_key) return;
                else break;
            }

            // Update anchor node if next edge to be traversed is a right edge
            if(which_edge == EdgeType::RIGHT) {
                anchor_record.node = cur;
                anchor_record.key_val = cur_key;
            }

            // Traverse the next edge
            p_last_edge = last_edge;
            last_edge = LFTreeEdge(cur, next, which_edge);
            cur = next;
        }

        // Key was not found; check if can stop
        LFTreeNode* anchor_node = anchor_record.node;
//        cout << anchor_node->right << endl;

//        cout << GET_DELETE_FLG(anchor_node->right) << " " << !GET_PROMOTE_FLG(anchor_node->right) << endl;
        if(!GET_DELETE_FLG(GET_RIGHT_CHILD(anchor_node)) && !GET_PROMOTE_FLG(GET_RIGHT_CHILD(anchor_node))) {
            // The anchor node is still part of the tree; check if the anchor node’s key has changed
            if(anchor_record.key_val == GET_KEY_VAL(anchor_node)) return;
        } else {
            // Check if the anchor record (the node and its key) matches that of the previous traversal
            if(prev_anchor_record == anchor_record) {
                // return the results of the previous traversal
                *seek_record = p_seek_record;
                return;
            }
        }

        // Store the results of the traversal and restart from root
        p_seek_record = *seek_record;
        prev_anchor_record = anchor_record;
    }
}


void LockFreeBST::reinitialize() {
    destroy(root_r);
    root_r = new LFTreeNode(INT_MAX - 2);
    root_s = new LFTreeNode(INT_MAX - 1);
    root_t = new LFTreeNode(INT_MAX);
    root_r->right = root_s;
    root_s->right = root_t;
}


void LockFreeBST::destroy(LFTreeNode *cur) {
    LFTreeNode* cur_node = GET_NODE_ADDR(cur);
    if(!cur_node) return;
    destroy(cur_node->left);
    destroy(cur_node->right);
    delete cur_node;
}


vector<int> LockFreeBST::trans2vec() {
    vector<int> res;
    trans2vec_helper(GET_LEFT_CHILD(root_t), res);
    return res;
}


void LockFreeBST::trans2vec_helper(LFTreeNode *cur, vector<int> &v) {
    if(!GET_NODE_ADDR(cur)) return;
    v.emplace_back(GET_KEY_VAL(cur));
    trans2vec_helper(GET_LEFT_CHILD(cur), v);
    trans2vec_helper(GET_RIGHT_CHILD(cur), v);
}
