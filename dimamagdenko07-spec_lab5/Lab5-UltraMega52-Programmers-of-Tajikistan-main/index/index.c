// Полностью на ваше усмотрение (только переиспользуйте код из предыдущих лабораторных, если он вам подходит)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "index.h"
#include "../rbtree/rbtree.h"
#include "../avl/avl.h"
#include "../btree/btree.h"
#include "../vector/generic.h"

#define MAX_TOKENS 512

Index* createIndex(TreeType type) {
    Index* empty_index = malloc(sizeof(Index));

    if (!empty_index) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    empty_index->type = type;
    if (type == TREE_RB) {
        empty_index->tree = createRBTree();
    } else if (type == TREE_BTREE) {
        empty_index->tree = createBTree();
    } else if (type == TREE_AVL) {
        empty_index->tree = createAVLTree();
    } else {
        empty_index->tree = NULL;
    }

    return empty_index;
}


void insertTerm(Index* idx, const char* term, int doc_id, const char* title) {
    if (!idx || !term || !title) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    if (idx->type == TREE_RB) {
        RBTree* my_rb = (RBTree*)idx->tree;
        rbInsert(my_rb, term, doc_id, title);
    }

    else if (idx->type == TREE_BTREE) {
        BTree* my_btree = (BTree*)idx->tree;
        btreeInsert(my_btree, term, doc_id, title);
    }

    else if (idx->type == TREE_AVL) {
        AVLTree* my_avl = (AVLTree*)idx->tree;
        avlInsert(my_avl, term, doc_id, title);
    }

    else {
        printf("Err in %s:\nInvalid data\n", __func__);
        return;
    }
}

Vector* lookupTerm(const Index* idx, const char* term) {
    if (!idx || !term) {
        printf("Err in %s:\nSome data is empty\n", __func__);
    }

    if (idx->type == TREE_RB) {
        const RBTree* my_rb = (const RBTree*)idx->tree;
        return rbSearch(my_rb, term);
    }
    else if (idx->type == TREE_BTREE) {
        const BTree* my_btree = (const BTree*)idx->tree;
        return btreeSearch(my_btree, term);
    }

    else if (idx->type == TREE_AVL) {
        const AVLTree* my_avl = (const AVLTree*)idx->tree;
        return avlSearch(my_avl, term);
    }

    else {
        printf("Err in %s:\nInvalid data\n", __func__);
        Vector *not_exist = createVector(1);
        return not_exist;
    }
}

static int tokenAlreadySeen(const char** tokens, int count, const char* token) {
    for (int i = 0; i < count; i++) {
        if (strcmp(tokens[i], token) == 0) {
            return 1;
        }
    }
    return 0;
}

void indexDocument(Index* idx, int doc_id, const char* title,
                   const char** tokens, int n_tokens)
{
    if (!idx || !title || !tokens || n_tokens <= 0) {
        return;
    }

    const char* unique_tokens[MAX_TOKENS];
    int unique_count = 0;

    for (int i = 0; i < n_tokens; i++) {
        if (!tokens[i] || tokens[i][0] == '\0') {
            continue;
        }

        if (tokenAlreadySeen(unique_tokens, unique_count, tokens[i])) {
            continue;
        }

        unique_tokens[unique_count++] = tokens[i];

        insertTerm(idx, tokens[i], doc_id, title);
    }
}

void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx) {

    if (!idx || !visit || !ctx) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    if (idx->type == TREE_RB) {
        const RBTree* my_rb = (const RBTree*)idx->tree;
        return rbTraverse(my_rb, visit, ctx);
    }
    else if (idx->type == TREE_BTREE) {
        const BTree* my_btree = (const BTree*)idx->tree;
        return btreeTraverse(my_btree, visit, ctx);
    }

    else if (idx->type == TREE_AVL) {
        const AVLTree* my_avl = (const AVLTree*)idx->tree;
        return avlTraverse(my_avl, visit, ctx);
    }

    else {
        printf("Err in %s:\nInvalid data\n", __func__);
        return;
    }
}


static void writeTerm(const char* key, Vector* postings, void* ctx) {
    FILE* file = (FILE*)ctx;

    if (!file || !key || !postings) {
        return;
    }

    fprintf(file, "TERM %s %zu\n", key, postings->size);

    for (size_t i = 0; i < postings->size; i++) {
        PostingEntry* entry = getVectorItem(postings, i);

        if (!entry) {
            continue;
        }

        fprintf(file, "POST %d %s\n", entry->doc_id, entry->title);
    }
}


void saveIndex(const Index* idx, const char* path) {
    if (!idx || !path) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    FILE* file = fopen(path, "w");

    if (!file) {
        printf("Err in %s:\nFailed to open file %s\n", __func__, path);
        return;
    }

    traverseIndex(idx, writeTerm, file);
    fclose(file);
}

Index* loadIndex(const char* path, TreeType type) {
    if (!path) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    FILE* file = fopen(path, "r");

    if (!file) {
        printf("Err in %s:\nFailed to open file %s\n", __func__, path);
        return NULL;
    }

    Index* idx = createIndex(type);

    if (!idx) {
        fclose(file);
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    char line[4096];
    char term[256];

    while (fgets(line, sizeof(line), file)) {
        size_t postings_count = 0;

        if (sscanf(line, "TERM %255s %zu", term, &postings_count) != 2) {
            continue;
        }

        for (size_t i = 0; i < postings_count; i++) {
            if (!fgets(line, sizeof(line), file)) {
                break;
            }

            int doc_id;
            char title[256];

            if (sscanf(line, "POST %d %[^\n]", &doc_id, title) != 2) {
                continue;
            }

            insertTerm(idx, term, doc_id, title);
        }
    }

    fclose(file);
    return idx;
}

void freeIndex(Index* idx) {
    if (!idx) return;

    if (idx->tree) {
        if (idx->type == TREE_RB) {
            freeRBTree((RBTree*)idx->tree);
        } else if (idx->type == TREE_BTREE) {
            freeBTree((BTree*)idx->tree);
        } else if (idx->type == TREE_AVL) {
            freeAVLTree((AVLTree*)idx->tree);
        }
    }

    free(idx);
}