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
//    cout << seek_record.last_edge.child->key.value << endl;
    if(seek_record.last_edge.child->key.value == v) return true;
    return false;
}


bool LockFreeBST::insert(int v) {
    // TODO: use one target record or individual seek record?
    seek(v, &target_record);
    LFTreeEdge target_edge = target_record.last_edge;
    LFTreeNode* node = target_edge.child;
    if(node->key.value == v) return false;  // Exists duplicate node

    // Create new node
    EdgeType which_edge  = target_record.inject_edge.type;
    LFTreeNode::LFNodeChild old_child(target_record.inject_edge.child, true);
    LFTreeNode::LFNodeChild new_child(new LFTreeNode(v), false);
//    LFTreeNode::LFNodeChild* new_child = new LFTreeNode::LFNodeChild(new LFTreeNode(v), false);
    LFTreeNode::LFNodeChild* target_addr = which_edge == EdgeType::LEFT ? node->left : node->right;
//    LFTreeNode::LFNodeChild** target_addr = which_edge == EdgeType::LEFT ? &node->left : &node->right;
    // TODO: implement CAS here (double)
//    bool result = __sync_bool_compare_and_swap(target_addr, old_child, new_child);
    return true;
    // TODO: helper function
    return false;
}


bool LockFreeBST::remove(int v) {
    return false;
}


void LockFreeBST::seek(int target_key, SeekRecord *seek_record) {
    AnchorRecord prev_anchor_record(root_s, root_s->key.value);
    SeekRecord p_seek_record;
    while(true) {
        // Initialize all variables used in traversal
        LFTreeEdge p_last_edge(root_r, root_s, EdgeType::RIGHT);
        LFTreeEdge last_edge(root_s, root_t, EdgeType::RIGHT);
        LFTreeNode *cur = root_t;
        AnchorRecord anchor_record(root_s, root_s->key.value);

        while(true) {
            int cur_key = cur->key.value;
            EdgeType which_edge = target_key > cur_key ? EdgeType::RIGHT : EdgeType::LEFT;
            LFTreeNode::LFNodeChild* child = (which_edge == EdgeType::LEFT ? cur->left : cur->right);
            LFTreeNode *next = child->child;

            // If either key found or no next edge to follow, stop the traversal
            if(cur_key == target_key || child->null_flg) {
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
        if(!anchor_node->right->delete_flg && !anchor_node->right->promote_flg) {
            // The anchor node is still part of the tree; check if the anchor node’s key has changed
            if(anchor_record.key_val == anchor_node->key.value) return;
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


//TODO
void LockFreeBST::reinitialize() {

}


void LockFreeBST::destroy(LFTreeNode *cur) {

}


vector<int> LockFreeBST::trans2vec() {

}


void LockFreeBST::trans2vec_helper(LFTreeNode *cur, vector<int> &v) {

}
