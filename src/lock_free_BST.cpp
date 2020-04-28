//
// Created by Zoeeeee on 4/24/20.
//

#include "lock_free_BST.h"
#define DEBUG 0

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
    SeekRecord target_record;
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

    // Help if needed
    LFTreeNode* node_child = which_edge == EdgeType::LEFT ? GET_LEFT_CHILD(node) : GET_RIGHT_CHILD(node);
    if(GET_DELETE_FLG(node_child)) help_target_node(target_edge);
    return false;
}


bool LockFreeBST::remove(int v) {
    SeekRecord target_record;
    StateRecord *myState = new StateRecord();
    int nkey = 0;
    // Initialize remove state record
    myState->target_key = v;
    myState->current_key = v;
    myState->mode = DelMode::INJECT;  // Start with inject mode

    while (true) {
        seek(myState->current_key, &target_record);
        LFTreeEdge target_edge = target_record.last_edge;
        LFTreeEdge p_target_edge = target_record.p_last_edge;
        nkey = GET_KEY_VAL(target_edge.child);      // Actual value of key
        if (myState->current_key != nkey) {
            // The key does not exist in the tree
            // Either not exist originally or deleted by other remove()
            return myState->mode != DelMode::INJECT;
        }

        // Perform appropriate action depending on the mode
        if (myState->mode == DelMode::INJECT) {
            // Store a reference to the target edge
            myState->target_edge = target_edge;
            myState->p_target_edge = p_target_edge;
            // Attempt to inject the operation at the node
            inject(myState);
        }

        // Mode would have changed if injection was successful
        if (myState->mode != DelMode::INJECT) {
            // Check if the target node found by the seek function
            // matches the one stored in the state record
            if (myState->target_edge.child != target_edge.child) {
                return true;
            }
            // Update the target edge information using the most recent seek
            myState->target_edge = target_edge;
        }

        if (myState->mode == DelMode::DISCOVERY) {
            // Complex delete operation; locate the successor node
            // and mark its child edges with promote flag
            find_and_mark_successor(myState);
        }

        if (myState->mode == DelMode::DISCOVERY) {
            // Complex delete operation; promote the successor
            // node’s key and remove the successor node
            remove_successor(myState);
        }

        if (myState->mode == DelMode::CLEANUP) {
            // Either remove the target node (simple delete) or replace
            // it with a new node with all fields unmarked (complex delete)
            if (cleanup(myState)) return true;
            nkey = GET_KEY_VAL(target_edge.child);
            myState->current_key = nkey;
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
    if(!cur_node || GET_NULL_FLG(cur)) return;
//    cout << cur << " " << GET_KEY_VAL(cur) << endl;
    destroy(cur_node->left);
    destroy(cur_node->right);
//    int c = GET_KEY_VAL(cur);
    delete cur_node;
//    cout << "finish " << c << endl;
}


vector<int> LockFreeBST::trans2vec() {
    vector<int> res;
    trans2vec_helper(GET_LEFT_CHILD(root_t), res);
    return res;
}


void LockFreeBST::trans2vec_helper(LFTreeNode *cur, vector<int> &v) {
    if(!GET_NODE_ADDR(cur) || GET_NULL_FLG(cur)) return;
    v.emplace_back(GET_KEY_VAL(cur));
    trans2vec_helper(GET_LEFT_CHILD(cur), v);
    trans2vec_helper(GET_RIGHT_CHILD(cur), v);
}

/******************* Functions called by remove operation *****************/
void LockFreeBST::inject(StateRecord *state) {
#if DEBUG
    cout << "begin inject" << endl;
#endif

    LFTreeEdge target_edge = state->target_edge;
    // Try to set the intent flag on the target edge
    // retrieve attributes of the target edge
    LFTreeNode *parent = target_edge.parent;
    LFTreeNode *node = target_edge.child;
    EdgeType which_edge = target_edge.type;

    LFTreeNode *old_node_ptr = node;
    RESET_ALL_NODEPTR_FLG(old_node_ptr);
    LFTreeNode *new_node_ptr = old_node_ptr;
    SET_INTENT_FLG(new_node_ptr);

    LFTreeNode **target_addr = which_edge == EdgeType::LEFT ? &(GET_NODE_ADDR(parent)->left) : &(GET_NODE_ADDR(parent)->right);
//    cout << *target_addr << " " << old_node_ptr << " " << new_node_ptr << endl;

    bool result = __sync_bool_compare_and_swap(target_addr, old_node_ptr, new_node_ptr);
    if (!result) {
        // Unable to set the intent flag; help if needed
        LFTreeNode *child_node_ptr = which_edge == EdgeType::LEFT ? GET_NODE_ADDR(parent)->left : GET_NODE_ADDR(parent)->right;
        if (GET_INTENT_FLG(child_node_ptr)) {
            help_target_node(target_edge);
        } else if (GET_DELETE_FLG(child_node_ptr)) {
            help_target_node(state->p_target_edge);
        } else if (GET_PROMOTE_FLG(child_node_ptr)) {
            help_successor_node(state->p_target_edge);
        }
        return;
    }

    // Mark the left edge for deletion
    result = mark_child_edge(state, EdgeType::LEFT);
    if (!result) return;
    // Mark the right edge for deletion; cannot fail
    mark_child_edge(state, EdgeType::RIGHT);

    // Initialize the type and mode of the operation
    initialize_type_and_update_mode(state);
}


void LockFreeBST::find_and_mark_successor(StateRecord *state) {
#if DEBUG
    cout << "begin find_and_mark_successor" << endl;
#endif

    // Retrieve the addresses from the state record
    LFTreeNode *node = state->target_edge.child;
    SeekRecord *seek_record = state->successorRecord;

    while (true) {
        // Find the node with the smallest key in the right subtree
        bool result = find_smallest(state);

        // Read the mark flag of the key in the target node
        if (GET_MODIFY_FLG(GET_NODE_ADDR(node)->key) || !result) {
            // Successor node had already been selected before
            // the traversal or the right subtree is empty
            break;
        }

        // Retrieve the information from the seek record
        LFTreeEdge successor_edge = seek_record->last_edge;
        LFTreeNode *left = seek_record->inject_edge.child;

        // Read the mark flag of the key under deletion
        if (GET_MODIFY_FLG(GET_NODE_ADDR(node)->key)) {  // Successor node has already been selected
            continue;
        }

        // Try to set the promote flag on the left edge
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

        // Attempt to mark the edge failed; recover from the failure and retry if needed
        LFTreeNode *left_child = GET_NODE_ADDR(successor_edge.child)->left;

        if (GET_NULL_FLG(left_child) && GET_DELETE_FLG(left_child)) {
            // The node found is undergoing deletion; need to help
            help_target_node(successor_edge);
        }
    }

    // Update the operation mode
    update_mode(state);
}


void LockFreeBST::remove_successor(StateRecord *state) {
#if DEBUG
    cout << "begin remove_successor" << endl;
#endif

    // Retrieve addresses from the state record
    LFTreeNode *node = state->target_edge.child;
    SeekRecord *seek_record = state->successorRecord;

    // Extract information about the successor node
    LFTreeEdge successor_edge = seek_record->last_edge;

    // Ascertain that the seek record for the successor node contains valid information
    LFTreeNode *left_child_addr = GET_LEFT_CHILD(successor_edge.child);

    if (!GET_PROMOTE_FLG(left_child_addr) || (GET_NODE_ADDR(left_child_addr) != node)) {
        node->ready_to_replace = true;
        update_mode(state);
        return;
    }

    // Mark the right edge for promotion if unmarked
    mark_child_edge(state, EdgeType::RIGHT);

    // Promote the key
    int *new_key_to_set = GET_KEY_ADDR(GET_NODE_ADDR(successor_edge.child)->key);
    SET_MODIFY_FLG(new_key_to_set);
    GET_NODE_ADDR(node)->key = new_key_to_set;

    bool set_del_flag;
    EdgeType which_edge;
    while (true) {
        // Check if the successor is the right child of the target node irself
        if (successor_edge.parent == node) {  // TODO: do we need to use macro here?
            // Need to modify the right edge of the target node whose delete flag is set
            set_del_flag = true;
            which_edge = EdgeType::RIGHT;
        } else {
            set_del_flag = false;
            which_edge = EdgeType ::LEFT;
        }
        LFTreeNode *which_child = which_edge == EdgeType::LEFT ? GET_LEFT_CHILD(successor_edge.parent) : GET_RIGHT_CHILD(successor_edge.parent);
        LFTreeNode *old_value = successor_edge.child;
        RESET_ALL_NODEPTR_FLG(old_value);
        if (set_del_flag) SET_DELETE_FLG(old_value);
        if (GET_INTENT_FLG(which_child)) SET_INTENT_FLG(old_value);

        LFTreeNode *right = GET_RIGHT_CHILD(successor_edge.child);
        LFTreeNode *new_value = nullptr;
        if (GET_NULL_FLG(right)) {
            // Only set the null flag; do not change the address
            new_value = successor_edge.child;
            RESET_ALL_NODEPTR_FLG(new_value);
            SET_NULL_FLG(new_value);
            if (set_del_flag) SET_DELETE_FLG(new_value);
        } else {
            // Switch the pointer to point to the grand child
            new_value = right;
            RESET_ALL_NODEPTR_FLG(new_value);
            if (set_del_flag) SET_DELETE_FLG(new_value);
        }

        LFTreeNode **target_addr = which_edge == EdgeType::LEFT ?
                &(GET_NODE_ADDR(successor_edge.parent)->left) : &(GET_NODE_ADDR(successor_edge.parent)->right);
        bool result = __sync_bool_compare_and_swap(target_addr, old_value, new_value);

        if (result || set_del_flag) break;

        LFTreeEdge p_last_edge = seek_record->p_last_edge;
        which_child = which_edge == EdgeType::LEFT ? GET_LEFT_CHILD(successor_edge.parent) : GET_RIGHT_CHILD(successor_edge.parent);
        if (GET_DELETE_FLG(which_child) && p_last_edge.parent != nullptr) {
            help_target_node(p_last_edge);
        }

        result = find_smallest(state);
        LFTreeEdge last_edge = seek_record->last_edge;
        if (!result || last_edge.child != successor_edge.child) {
            // The successor node has already been removed
            break;
        } else {
            successor_edge = seek_record->last_edge;
        }
        node->ready_to_replace = true;
        update_mode(state);
    }

}


bool LockFreeBST::cleanup(StateRecord *state) {
#if DEBUG
    cout << "begin cleanup" << endl;
#endif

    LFTreeEdge target_edge = state->target_edge;
    LFTreeNode *parent = target_edge.parent;
    LFTreeNode *node = target_edge.child;
    EdgeType p_which = target_edge.type;

    LFTreeNode *old_value = nullptr;
    LFTreeNode *new_value = nullptr;

    if (state->type == DelType::COMPLEX) {
        // Replace the node with a new copy in which all fields are unmarked
        LFTreeNode *new_node = new LFTreeNode();
        int *new_key = GET_NODE_ADDR(node)->key;
        RESET_MODIFY_FLG(new_key);
        GET_NODE_ADDR(new_node)->key = new_key;

        // Initialize left and right child pointers
        LFTreeNode *left = GET_LEFT_CHILD(node);
        RESET_ALL_NODEPTR_FLG(left);
        GET_NODE_ADDR(new_node)->left = left;

        LFTreeNode *right = GET_RIGHT_CHILD(node);
        if (GET_NULL_FLG(right)) {
            right = nullptr;
            SET_NULL_FLG(right);
        } else {
            RESET_ALL_NODEPTR_FLG(right);
        }
        GET_NODE_ADDR(new_node)->right = right;

        // Initialize the arguments of CAS instruction
        LFTreeNode *old_value = node;
        RESET_ALL_NODEPTR_FLG(old_value);
        SET_INTENT_FLG(old_value);
        LFTreeNode *new_value = new_node;
        RESET_ALL_NODEPTR_FLG(new_value);
    } else {  // Remove the node
        // Determine to which grand child will the edge at the parent be switched
        EdgeType n_which = (GET_NULL_FLG(GET_LEFT_CHILD(node))) ? EdgeType::RIGHT : EdgeType::LEFT;

        // Initialize the arguments of the CAS instruction
        old_value = node;
        RESET_ALL_NODEPTR_FLG(old_value);
        SET_INTENT_FLG(old_value);

        LFTreeNode *address = (n_which == EdgeType::LEFT) ? GET_LEFT_CHILD(node) : GET_RIGHT_CHILD(node);
        if (GET_NULL_FLG(address)) {  // Set the null flag only
            new_value = node;
            RESET_ALL_NODEPTR_FLG(new_value);
            SET_NULL_FLG(new_value);
        } else {  // Change the pointer to the grant child
            new_value = address;
            RESET_ALL_NODEPTR_FLG(new_value);
        }
    }

    LFTreeNode **target_addr = (p_which == EdgeType::LEFT) ? &(GET_NODE_ADDR(parent)->left) : &(GET_NODE_ADDR(parent)->right);
//    cout << *target_addr << " " << old_value << " " << new_value << endl;
    bool result = __sync_bool_compare_and_swap(target_addr, old_value, new_value);
    return result;
}


/*** Help routine functions ***/
// TODO: Implement mark_child_edge
bool LockFreeBST::mark_child_edge(StateRecord *state, EdgeType which_edge) {
#if DEBUG
    cout << "begin mark_child_edge" << endl;
#endif

    unsigned long flg = 0;
    LFTreeEdge edge;
    if(state->mode == DelMode::INJECT) {
        edge = state->target_edge;
        flg = DELETE_BIT;
    } else {
        edge = state->successorRecord->last_edge;
        flg = PROMOTE_BIT;
    }
    LFTreeNode* node = edge.child;

    while(true) {
        LFTreeNode* child = which_edge == EdgeType::LEFT ? GET_LEFT_CHILD(node) : GET_RIGHT_CHILD(node);
        if(GET_INTENT_FLG(child)) {
            help_target_node(LFTreeEdge(node, GET_NODE_ADDR(child), which_edge));
            continue;
        } else if(GET_DELETE_FLG(child)) {
            if(flg == DELETE_BIT) {
                help_target_node(edge);
                return false;
            } else return true;
        } else if(GET_PROMOTE_FLG(child)) {
            if(flg == DELETE_BIT) {
                help_successor_node(edge);
                return false;
            } else return true;
        }
        LFTreeNode* old_value = GET_NODE_ADDR(child);
        RESET_ALL_NODEPTR_FLG(old_value);
        if(GET_NULL_FLG(child)) SET_NULL_FLG(old_value);
        LFTreeNode* new_value = NODE_PTR(LNG(old_value) | flg);
        LFTreeNode** target_addr = which_edge == EdgeType::LEFT ? &(GET_NODE_ADDR(node)->left) : &(GET_NODE_ADDR(node)->right);
        bool result = __sync_bool_compare_and_swap(target_addr, old_value, new_value);
        if(result) break;
    }
    return true;
}


// TODO: Implement mark_child_edge
bool LockFreeBST::find_smallest(StateRecord *state) {
#if DEBUG
    cout << "begin find_smallest" << endl;
#endif

    // Find the node with the smallest key in the subtree
    // rooted at the right child of the target node.
    LFTreeNode* node = state->target_edge.child;
    SeekRecord* seek_record = state->successorRecord;
    LFTreeNode* right_child = GET_RIGHT_CHILD(node);
    if(GET_NULL_FLG(right_child)) return false; // The right subtree is empty

    // Initialize the variables used in the traversal
    LFTreeEdge last_edge(node, GET_NODE_ADDR(right_child), EdgeType::RIGHT);
    LFTreeEdge p_last_edge(node, GET_NODE_ADDR(right_child), EdgeType::RIGHT);
    LFTreeEdge inject_edge;
    while(true) {
        LFTreeNode* cur = last_edge.child;
        LFTreeNode* left_child = GET_LEFT_CHILD(cur);
        if(GET_NULL_FLG(left_child)) {
            inject_edge = LFTreeEdge(cur, GET_NODE_ADDR(left_child), EdgeType::LEFT);
            break;
        }

        // Traverse the next edge
        p_last_edge = last_edge;
        last_edge = LFTreeEdge(cur, GET_NODE_ADDR(left_child), EdgeType::LEFT);
    }

    // Initialize seek record and return
    seek_record->last_edge = last_edge;
    seek_record->p_last_edge = p_last_edge;
    seek_record->inject_edge = inject_edge;
    return true;
}


void LockFreeBST::initialize_type_and_update_mode(StateRecord *state) {
#if DEBUG
    cout << "begin initialize_type_and_update_mode" << endl;
#endif

    // retrieve the target node's address from the state record
    LFTreeNode *node = state->target_edge.child;

    LFTreeNode *left_child = GET_LEFT_CHILD(node);
    LFTreeNode *right_child = GET_RIGHT_CHILD(node);
    if (GET_NULL_FLG(left_child) || GET_NULL_FLG(right_child)) {
        // One of the child pointers is null
        state->type = (GET_MODIFY_FLG(GET_NODE_ADDR(node)->key)) ? DelType::COMPLEX : DelType::SIMPLE;
    } else {  // Both child pointers are non-null
        state->type = DelType::COMPLEX;
    }
    update_mode(state);
}


void LockFreeBST::update_mode(StateRecord *state) {
#if DEBUG
    cout << "begin update_mode" << endl;
#endif

    if (state->type == DelType::SIMPLE) { // Simple delete
        state->mode = DelMode::CLEANUP;
    } else {  // Complex delete
        LFTreeNode *node = state->target_edge.child;
        state->mode = (GET_NODE_ADDR(node)->ready_to_replace) ? DelMode::CLEANUP : DelMode::DISCOVERY;
    }
}


/*** Helping conflicting delete operation ***/
void LockFreeBST::help_target_node(LFTreeEdge helpee_edge) {
#if DEBUG
    cout << "help_target_node" << endl;
#endif

    StateRecord* state = new StateRecord();
    // Intent flag must be set on the edge
    // obtain new state record and init it
    state->target_edge = helpee_edge;
    state->mode = DelMode::INJECT;

    // Mark the left and right edges if unmarked
    bool result = mark_child_edge(state, EdgeType::LEFT);
    if (!result) return;
    mark_child_edge(state, EdgeType::RIGHT);
    initialize_type_and_update_mode(state);

    // Perform the remaining steps of a delete operation
    if (state->mode == DelMode::DISCOVERY) {
        find_and_mark_successor(state);
    }

    if (state->mode == DelMode::DISCOVERY) {
        remove_successor(state);
    }

    if (state->mode == DelMode::CLEANUP) {
        cleanup(state);
    }
}


void LockFreeBST::help_successor_node(LFTreeEdge helpee_edge) {
#if DEBUG
    cout << "help_successor_node" << endl;
#endif

    // Retrieve the address of the successor node
    LFTreeNode *parent = helpee_edge.parent;
    LFTreeNode *node = helpee_edge.child;
    // Promote flat must be set on the successor node's left edge
    // retrieve the address of the target node
    LFTreeNode *left = GET_LEFT_CHILD(node);
    RESET_ALL_NODEPTR_FLG(left);

    // Obtain new state record and initialize it
    StateRecord* state = new StateRecord();
    state->target_edge = LFTreeEdge(nullptr, left, EdgeType::INIT);
    state->mode = DelMode::DISCOVERY;

    // TODO: WTF? state->successorRecord would be nullptr in this case
    SeekRecord *seek_record = state->successorRecord;
    // Initialize the seek record in the state record
    seek_record->last_edge = helpee_edge;
    seek_record->p_last_edge = LFTreeEdge(nullptr, parent, EdgeType::INIT);

    // Promote the successor node's key and remove the successor node
    remove_successor(state);

    // No need to perform the cleanup
}

