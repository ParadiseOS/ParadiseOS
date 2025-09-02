#include "rb_tree.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/types.h"
#include "terminal/terminal.h"

// See https://dl.ebooksworld.ir/books/Introduction.to.Algorithms.4th.Leiserson.Stein.Rivest.Cormen.MIT.Press.9780262046305.EBooksWorld.ir.pdf

#define RED   0
#define BLACK 1

#define LEFT  0
#define RIGHT 1

#define NIL(node) ((node) == &tree->nil)

static u8 direction(RbNode *node) {
    KERNEL_ASSERT(node->parent);
    return node == node->parent->left ? LEFT : RIGHT;
}

static void rotate_subtree(RbTree *tree, RbNode *node, u8 dir) {
    RbNode *sub = node->child[1 - dir];

    node->child[1 - dir] = sub->child[dir];
    if (!NIL(sub->child[dir]))
        sub->child[dir]->parent = node;
    sub->parent = node->parent;

    if (NIL(node->parent))
        tree->root = sub;
    else
        node->parent->child[direction(node)] = sub;

    sub->child[dir] = node;
    node->parent = sub;
}

static void insert_fix(RbTree *tree, RbNode *node) {
    while (node->parent->color == RED) {
        u8 pinv = 1 - direction(node->parent);
        RbNode *uncle = node->parent->parent->child[pinv];
        if (uncle->color == RED) {
            node->parent->color = BLACK;
            uncle->color = BLACK;
            node->parent->parent->color = RED;
            node = node->parent->parent;
        }
        else {
            u8 dir = direction(node);
            if (dir == pinv) {
                node = node->parent;
                rotate_subtree(tree, node, 1 - pinv);
            }

            node->parent->color = BLACK;
            node->parent->parent->color = RED;
            rotate_subtree(tree, node->parent->parent, pinv);
        }
    }

    tree->root->color = BLACK;
}

static void remove_fix(RbTree *tree, RbNode *node) {
    while (node != tree->root && node->color == BLACK) {
        u8 inv = 1 - direction(node);
        RbNode *sibling = node->parent->child[inv];
        if (sibling->color == RED) {
            sibling->color = BLACK;
            node->parent->color = RED;
            rotate_subtree(tree, node->parent, 1 - inv);
            sibling = node->parent->child[inv];
        }
        if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
            sibling->color = RED;
            node = node->parent;
        }
        else {
            if (sibling->child[inv]->color == BLACK) {
                sibling->child[1 - inv]->color = BLACK;
                sibling->color = RED;
                rotate_subtree(tree, sibling, inv);
                sibling = node->parent->child[inv];
            }

            sibling->color = node->parent->color;
            node->parent->color = BLACK;
            sibling->child[inv]->color = BLACK;
            rotate_subtree(tree, node->parent, 1 - inv);
            node = tree->root;
        }
    }

    node->color = BLACK;
}

static void transplant(RbTree *tree, RbNode *a, RbNode *b) {
    if (NIL(a->parent))
        tree->root = b;
    else
        a->parent->child[direction(a)] = b;
    b->parent = a->parent;
}

static RbNode *minimum(RbTree *tree, RbNode *node) {
    while (!NIL(node->left))
        node = node->left;
    return node;
}

void rb_init(RbTree *tree) {
    tree->root = &tree->nil;
    tree->nil.left = &tree->nil;
    tree->nil.right = &tree->nil;
    tree->nil.color = BLACK;
    tree->count = 0;
}

bool rb_insert(RbTree *tree, RbNode *node, u16 key) {
    RbNode *loc = tree->root;
    RbNode *parent = &tree->nil;
    u8 dir;

    while (!NIL(loc)) {
        if (loc->key == key)
            return false;
        dir = key > loc->key;
        parent = loc;
        loc = loc->child[dir];
    }

    node->parent = parent;
    node->key = key;
    node->left = &tree->nil;
    node->right = &tree->nil;
    node->color = RED;

    if (NIL(parent))
        tree->root = node;
    else
        parent->child[dir] = node;

    tree->count += 1;
    insert_fix(tree, node);
    return true;
}

RbNode *rb_remove(RbTree *tree, u16 key) {
    RbNode *node = rb_find(tree, key);

    if (!node)
        return NULL;

    RbNode *x = NULL;
    RbNode *y = node;
    u8 y_color = y->color;

    if (NIL(node->left) || NIL(node->right)) {
        u8 dir = NIL(node->left) ? RIGHT : LEFT;
        x = node->child[dir];
        transplant(tree, node, x);
    }
    else {
        y = minimum(tree, node->right);
        y_color = y->color;
        x = y->right;

        if (y != node->right) {
            transplant(tree, y, y->right);
            y->right = node->right;
            y->right->parent = y;
        }
        else {
            x->parent = y;
        }

        transplant(tree, node, y);
        y->left = node->left;
        y->left->parent = y;
        y->color = node->color;
    }

    if (y_color == BLACK)
        remove_fix(tree, x);

    tree->count -= 1;
    return node;
}

RbNode *rb_find(RbTree *tree, u16 key) {
    RbNode *node = tree->root;

    while (!NIL(node)) {
        if (node->key == key)
            return node;
        node = node->child[key > node->key];
    }

    return NULL;
}
