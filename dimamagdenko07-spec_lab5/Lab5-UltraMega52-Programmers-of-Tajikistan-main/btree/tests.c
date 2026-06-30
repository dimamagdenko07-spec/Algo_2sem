#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "btree.h"

#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_CYAN    "\x1b[36m"

int total_passed = 0;
int total_tests = 0;

#define RUN_CHECK(condition, message) \
    do { \
        total_tests++; \
        if (condition) { \
            printf("  " COLOR_GREEN "[OK]" COLOR_RESET "   %s\n", message); \
            total_passed++; \
        } else { \
            printf("  " COLOR_RED "[FAIL]" COLOR_RESET " %s\n", message); \
        } \
    } while(0)

typedef struct {
    int count;
    char last_key[256];
    bool is_sorted;
} TraverseContext;

void test_visit_callback(const char* key, Vector* postings, void* ctx) {
    TraverseContext* tctx = (TraverseContext*)ctx;
    
    if (tctx->count > 0 && strcmp(tctx->last_key, key) >= 0) {
        tctx->is_sorted = false;
    }
    
    strcpy(tctx->last_key, key);
    tctx->count++;
}

int main() {
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Initialization: Creating empty tree (createBTree)\n");
    BTree* tree1 = createBTree();
    RUN_CHECK(tree1 != NULL, "BTree structure allocated successfully");
    RUN_CHECK(tree1->root != NULL, "Root node initialized");
    RUN_CHECK(tree1->size == 0, "Initial tree size is 0");
    RUN_CHECK(tree1->root->is_leaf == 1, "Root is marked as a leaf node");
    freeBTree(tree1);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Insertion and Splitting: Growing the tree (btreeInsert)\n");
    BTree* tree2 = createBTree();
    
    btreeInsert(tree2, "apple", 1, "Doc1");
    btreeInsert(tree2, "banana", 1, "Doc1");
    btreeInsert(tree2, "cherry", 1, "Doc1");
    btreeInsert(tree2, "date", 1, "Doc1");
    btreeInsert(tree2, "elderberry", 1, "Doc1");
    btreeInsert(tree2, "fig", 1, "Doc1");
    btreeInsert(tree2, "grape", 1, "Doc1");

    RUN_CHECK(tree2->size == 7, "Tree size updated correctly to 7 unique keys");
    RUN_CHECK(tree2->root->is_leaf == 0, "Root split successfully (is no longer a leaf)");
    RUN_CHECK(tree2->root->n > 0, "New root contains median keys after split");
    freeBTree(tree2);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Lookup: Searching for terms (btreeSearch)\n");
    BTree* tree3 = createBTree();
    btreeInsert(tree3, "cat", 10, "Animals");
    btreeInsert(tree3, "dog", 11, "Animals");
    btreeInsert(tree3, "cat", 12, "Pets");

    Vector* found_cat = btreeSearch(tree3, "cat");
    Vector* found_dog = btreeSearch(tree3, "dog");
    Vector* found_bird = btreeSearch(tree3, "bird");

    RUN_CHECK(found_cat != NULL, "Existing key 'cat' found successfully");
    RUN_CHECK(found_dog != NULL, "Existing key 'dog' found successfully");
    RUN_CHECK(found_bird == NULL, "Missing key 'bird' correctly returned NULL");
    
    freeBTree(tree3);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Traversal: In-order tree walk (btreeTraverse)\n");
    BTree* tree4 = createBTree();
    
    btreeInsert(tree4, "zebra", 1, "Doc");
    btreeInsert(tree4, "apple", 1, "Doc");
    btreeInsert(tree4, "monkey", 1, "Doc");
    btreeInsert(tree4, "bear", 1, "Doc");
    btreeInsert(tree4, "lion", 1, "Doc");

    TraverseContext ctx = { .count = 0, .last_key = "", .is_sorted = true };
    btreeTraverse(tree4, test_visit_callback, &ctx);

    RUN_CHECK(ctx.count == 5, "All 5 nodes were visited by the callback");
    RUN_CHECK(ctx.is_sorted == true, "Keys were strictly in alphabetical order during traversal");
    freeBTree(tree4);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Memory Management: Clearing massive data (freeBTree)\n");
    BTree* tree5 = createBTree();
    char buffer[32];
    for (int i = 0; i < 150; i++) {
        sprintf(buffer, "word_number_%d", i);
        btreeInsert(tree5, buffer, i, "LoadTest");
    }
    RUN_CHECK(tree5->size == 150, "Successfully inserted 150 elements (forced multiple splits)");
    
    freeBTree(tree5); 
    RUN_CHECK(true, "Memory cleanup executed without segmentation fault");
    printf("\n");

    printf(COLOR_CYAN "=================================================================" COLOR_RESET "\n");
    printf(COLOR_CYAN "TOTAL: Passed %d out of %d tests" COLOR_RESET "\n", total_passed, total_tests);
    
    if (total_passed == total_tests) {
        printf(COLOR_GREEN "RESULT: SUCCESS" COLOR_RESET "\n");
    } else {
        printf(COLOR_RED "RESULT: FAILURE" COLOR_RESET "\n");
    }
    printf(COLOR_CYAN "=================================================================" COLOR_RESET "\n\n");

    return (total_passed == total_tests) ? 0 : 1;
}