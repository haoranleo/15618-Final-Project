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
    int nkey = 0;
    StateRecord *state = new StateRecord();
    // Initialize remove state record
    state->target_key = v;
    state->current_key = v;
    state->mode = DelMode::INJECT;  // Start with inject mode

    while (true) {
        seek(state->target_key, &target_record);
        LFTreeEdge target_edge = target_record.last_edge;
        LFTreeEdge p_target_edge = target_record.p_last_edge;
        nkey = *(GET_KEY_ADDR(target_edge.child->key));  // Actual value of key
        if (state->current_key != nkey) {
            // the key does not exist in the tree
            // Either not exist originally or deleted by other remove()
            return state->mode != DelMode::INJECT;
        }

        // perform appropriate action depending on the mode
        if (state->mode == DelMode::INJECT) {
            // store a reference to the target edge
            state->target_edge = target_edge;
            state->p_target_edge = p_target_edge;
            // attempt to inject the operation at the node
            inject(state);
        }

        // mode would have changed if injection was successful
        if (state->mode != DelMode::INJECT) {
            // check if the target node found by the seek function
            // matches the one stored in the state record
            if (state->target_edge.child != target_edge.child) {
                return true;
            }
            // update the target edge information using the most recent seek
            state->target_edge = target_edge;
        }

        if (state->mode == DelMode::DISCOVERY) {
            // complex delete operation; locate the successor node
            // and mark its child edges with promote flag
            find_and_mark_successor(state);
        }

        if (state->mode == DelMode::DISCOVERY) {
            // complex delete operation; promote the successor
            // node’s key and remove the successor node
            remove_successor(state);
        }

        if (state->mode == DelMode::CLEANUP) {
            // either remove the target node (simple delete) or
            // replace it with a new node with all fields unmarked
            // (complex delete)
            bool result = cleanup(state);
            if (result) {
                return true;
            }
            nkey = *(GET_KEY_ADDR(target_edge.child->key));
            state->current_key = nkey;
        }
    }
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

/******************* Functions called by remove operation *****************/
void inject(StateRecord *state) {
    LFTreeEdge target_edge = state->target_edge;
    // try to set the intent flag on the target edge
    // retrieve attributes of the target edge
    LFTreeNode *parent = target_edge.parent;
    LFTreeNode *node = target_edge.child;
    EdgeType which_edge = target_edge.type;

    LFTreeNode *old_node_ptr = node;
    RESET_ALL_NODEPTR_FLG(old_node_ptr);
    LFTreeNode *new_node_ptr = node;
    SET_INTENT_FLG(new_node_ptr);

    LFTreeNode **target_addr = which_edge == EdgeType::LEFT ? &(GET_NODE_ADDR(parent)->left) : &(GET_NODE_ADDR(parent)->right);
    cout << *target_addr << " " << old_node_ptr << " " << new_node_ptr << endl;

    bool result = __sync_bool_compare_and_swap(target_addr, old_node_ptr, new_node_ptr);
    if (!result) {
        // unable to set the intent flag; help if needed
        LFTreeNode *child_node_ptr = which_edge == EdgeType::LEFT ? (GET_NODE_ADDR(parent)->left) : (GET_NODE_ADDR(parent)->right);
        if (GET_INTENT_FLG(child_node_ptr)) {
            help_target_node(target_edge);
        } else if (GET_DELETE_FLG(child_node_ptr)) {
            help_target_node(state->p_target_edge);
        } else if (GET_PROMOTE_FLG(child_node_ptr)) {
            help_successor_node(state->p_target_edge);
        }
        return;
    }

    // mark the left edge for deletion
    result = mark_child_edge(state, EdgeType::LEFT);
    if (!result) return;
    // mark the right edge for deletion; cannot fail
    mark_child_edge(state, EdgeType::RIGHT);

    // initialize the type and mode of the operation
    initialize_type_and_update_mode(state);
}


void find_and_mark_successor(StateRecord *state) {
    // retrieve the addresses from the state record
    LFTreeNode *node = state->target_edge.child;
    SeekRecord *seek_record = state->successorRecord;

    while (true) {
        // find the node with the smallest key in the right subtree
        bool result = find_smallest(state);
        // read the mark flag of the key in the target node
        if (GET_MODIFY_FLG(node->key) || !result) {
            // successor node had already been selected before
            // the traversal or the right subtree is empty
            break;
        }

        // retrieve the information from the seek record
        LFTreeEdge successor_edge = seek_record->last_edge;
        LFTreeNode *left = seek_record->inject_edge.child;

        // read the mark flag of the key under deletion
        if (GET_MODIFY_FLG(node)) {  // successor node has already been selected
            continue;
        }

        // try to set the promote flag on the left edge
        LFTreeNode **target_addr = &(GET_NODE_ADDR(successor_edge.child)->left);
        LFTreeNode *old_node_ptr = left;
        RESET_ALL_NODEPTR_FLG(old_node_ptr);
        SET_NULL_FLG(old_node_ptr);
        LFTreeNode *new_node_ptr = node;
        RESET_ALL_NODEPTR_FLG(new_node_ptr);
        SET_NULL_FLG(new_node_ptr);
        SET_PROMOTE_FLG(new_node_ptr);

        result = __sync_bool_compare_and_swap(target_addr, old_node_ptr, new_node_ptr);
        if (result) break;

        // attempt to mark the edge failed; recover from the failure and retry if needed
        LFTreeNode *left_child = GET_NODE_ADDR(successor_edge.child)->left;

        if (GET_NULL_FLG(left_child) && GET_DELETE_FLG(left_child)) {
            // the node found is undergoing deletion; need to help
            // TODO: implement help target node
            help_target_node(successor_edge);
        }
    }

    // update the operation mode
    update_mode(state);
}


// TODO: Implement remove successor
void remove_successor(StateRecord *state) {
    cout << "NOT IMPLEMENTED: remove successor" << endl;
    return;
}


bool cleanup(StateRecord *state) {
    cout << "NOT IMPLEMENTED: cleanup" << endl;
    return false;
}


/*** Help routine functions ***/
bool mark_child_edge(StateRecord *state, EdgeType which_edge) {
    cout << "NOT IMPLEMENTED: mark child edge" << endl;
    return false;
}


bool find_smallest(StateRecord *state) {
    cout << "NOT IMPLEMENTED: find smallest" << endl;
    return false;
}


void initialize_type_and_update_mode(StateRecord *state) {
    cout << "NOT IMPLEMENTED: init type and update mode" << endl;
    return;
}


void update_mode(StateRecord *state) {
    cout << "NOT IMPLEMENTED: update mode" << endl;
    return;
}


/*** Helping conflicting delte operation ***/
void help_target_node(LFTreeEdge helpee_edge) {
    cout << "NOT IMPLEMENTED: help target node" << endl;
    return;
}

void help_successor_node(LFTreeEdge helpee_edge) {
    cout << "NOT IMPLEMENTED: help successor node" << endl;
    return;
}

