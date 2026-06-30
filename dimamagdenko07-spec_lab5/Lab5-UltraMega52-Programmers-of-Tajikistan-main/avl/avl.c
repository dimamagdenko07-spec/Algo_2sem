#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "avl.h"

static int max_int(int a, int b) {
    return a > b ? a : b;
}

static int node_height(AVLNode* node) {
    if (!node) {
        return 0;
    }

    return node->height;
}

static int balance_factor(AVLNode* node) {
    if (!node) {
        return 0;
    }

    return node_height(node->left) - node_height(node->right);
}

static char* copy_str(const char* src) {
    if (!src) {
        return NULL;
    }

    size_t len = strlen(src);
    char* dst = malloc(len + 1);

    if (!dst) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    strcpy(dst, src);
    return dst;
}

static AVLNode* create_node(const char* key, int doc_id, const char* title) {
    AVLNode* node = malloc(sizeof(AVLNode));

    if (!node) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    node->key = copy_str(key);

    if (!node->key) {
        free(node);
        return NULL;
    }

    node->postings = createPostingList();

    if (!node->postings) {
        free(node->key);
        free(node);
        return NULL;
    }

    appendPosting(node->postings, doc_id, title);

    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* t2 = x->right;

    x->right = y;
    y->left = t2;

    y->height = max_int(node_height(y->left), node_height(y->right)) + 1;
    x->height = max_int(node_height(x->left), node_height(x->right)) + 1;

    return x;
}

static AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* t2 = y->left;

    y->left = x;
    x->right = t2;

    x->height = max_int(node_height(x->left), node_height(x->right)) + 1;
    y->height = max_int(node_height(y->left), node_height(y->right)) + 1;

    return y;
}

static AVLNode* insert_node(AVLNode* node, const char* key, int doc_id, const char* title, int* is_new) {
    if (!node) {
        AVLNode* new_node = create_node(key, doc_id, title);

        if (new_node && is_new) {
            *is_new = 1;
        }

        return new_node;
    }

    int cmp = strcmp(key, node->key);

    if (cmp < 0) {
        node->left = insert_node(node->left, key, doc_id, title, is_new);
    } else if (cmp > 0) {
        node->right = insert_node(node->right, key, doc_id, title, is_new);
    } else {
        appendPosting(node->postings, doc_id, title);
        return node;
    }

    node->height = max_int(node_height(node->left), node_height(node->right)) + 1;

    int balance = balance_factor(node);

    if (balance > 1 && strcmp(key, node->left->key) < 0) {
        return right_rotate(node);
    }

    if (balance < -1 && strcmp(key, node->right->key) > 0) {
        return left_rotate(node);
    }

    if (balance > 1 && strcmp(key, node->left->key) > 0) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && strcmp(key, node->right->key) < 0) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

static void free_node(AVLNode* node) {
    if (!node) {
        return;
    }

    free_node(node->left);
    free_node(node->right);

    free(node->key);
    vectorFree(node->postings);
    free(node);
}

static Vector* search_node(const AVLNode* node, const char* key) {
    if (!node) {
        return NULL;
    }

    int cmp = strcmp(key, node->key);

    if (cmp == 0) {
        return node->postings;
    }

    if (cmp < 0) {
        return search_node(node->left, key);
    }

    return search_node(node->right, key);
}

static void traverse_node(
    AVLNode* node,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!node) {
        return;
    }

    traverse_node(node->left, visit, ctx);
    visit(node->key, node->postings, ctx);
    traverse_node(node->right, visit, ctx);
}

AVLTree* createAVLTree(void) {
    AVLTree* tree = malloc(sizeof(AVLTree));

    if (!tree) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    tree->root = NULL;
    tree->size = 0;

    return tree;
}

void freeAVLTree(AVLTree* tree) {
    if (!tree) {
        return;
    }

    free_node(tree->root);
    free(tree);
}

void avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !key || !title || strlen(key) == 0) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    int is_new = 0;
    tree->root = insert_node(tree->root, key, doc_id, title, &is_new);

    if (is_new) {
        tree->size++;
    }
}

Vector* avlSearch(const AVLTree* tree, const char* key) {
    if (!tree || !key || strlen(key) == 0) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    return search_node(tree->root, key);
}

void avlTraverse(
    const AVLTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!tree || !visit) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    traverse_node(tree->root, visit, ctx);
}