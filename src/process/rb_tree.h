#ifndef RB_TREE_H_
#define RB_TREE_H_

#include "lib/types.h"

typedef struct RbNode_ RbNode;
struct RbNode_ {
    RbNode *parent;
    union {
        struct {
            RbNode *left;
            RbNode *right;
        };

        RbNode *child[2];
    };
    u32 key;
    u8 color;
};

typedef struct {
    RbNode *root;
    RbNode nil;
    u32 count;
} RbTree;

// Note that the caller always owns the node memory. It should be pre-allocated
// for insertion.

void rb_init(RbTree *tree);

// Returns whether or not the insertion occurred. Insertion occurs if and only
// if the key does not already exist.
bool rb_insert(RbTree *tree, RbNode *node, u32 key);
RbNode *rb_find(RbTree *tree, u32 key);
RbNode *rb_remove(RbTree *tree, u32 key);

#endif // RB_TREE_H_
