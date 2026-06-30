#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "avl.h"

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
    const char** expected;
    int index;
    bool ok;
} TraverseCtx;

void visit_check_order(const char* key, Vector* postings, void* ctx) {
    TraverseCtx* data = (TraverseCtx*)ctx;

    if (strcmp(key, data->expected[data->index]) != 0) {
        data->ok = false;
    }

    if (postings == NULL) {
        data->ok = false;
    }

    data->index++;
}

int main() {
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Initialization: Creating empty AVL tree (createAVLTree)\n");
    AVLTree* tree1 = createAVLTree();

    RUN_CHECK(tree1 != NULL, "AVLTree structure allocated successfully");
    RUN_CHECK(tree1->root == NULL, "Root initialized as NULL");
    RUN_CHECK(tree1->size == 0, "Initial tree size is 0");

    freeAVLTree(tree1);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Insertion: Single key insertion (avlInsert)\n");
    AVLTree* tree2 = createAVLTree();

    avlInsert(tree2, "python", 1, "Python title");

    RUN_CHECK(tree2->root != NULL, "Root node created successfully");
    RUN_CHECK(strcmp(tree2->root->key, "python") == 0, "Root key is 'python'");
    RUN_CHECK(tree2->size == 1, "Tree size updated to 1");
    RUN_CHECK(tree2->root->height == 1, "Root height is 1");

    freeAVLTree(tree2);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Lookup: Searching existing and missing keys (avlSearch)\n");
    AVLTree* tree3 = createAVLTree();

    avlInsert(tree3, "python", 1, "Python title");

    Vector* found_python = avlSearch(tree3, "python");
    Vector* found_java = avlSearch(tree3, "java");

    RUN_CHECK(found_python != NULL, "Existing key 'python' found successfully");
    RUN_CHECK(found_python != NULL && found_python->size == 1, "Posting list for 'python' contains 1 entry");
    RUN_CHECK(found_java == NULL, "Missing key 'java' correctly returned NULL");

    if (found_python != NULL) {
        PostingEntry* entry = getVectorItem(found_python, 0);

        RUN_CHECK(entry != NULL, "Posting entry retrieved successfully");
        RUN_CHECK(entry != NULL && entry->doc_id == 1, "Posting doc_id is correct");
        RUN_CHECK(entry != NULL && strcmp(entry->title, "Python title") == 0, "Posting title is correct");
    }

    freeAVLTree(tree3);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Duplicates: Adding postings to existing key (avlInsert)\n");
    AVLTree* tree4 = createAVLTree();

    avlInsert(tree4, "python", 1, "First title");
    avlInsert(tree4, "python", 2, "Second title");

    Vector* postings = avlSearch(tree4, "python");

    RUN_CHECK(tree4->size == 1, "Duplicate key does not increase unique key count");
    RUN_CHECK(postings != NULL, "Posting list for duplicate key exists");
    RUN_CHECK(postings != NULL && postings->size == 2, "Duplicate key stores 2 postings");

    if (postings != NULL && postings->size == 2) {
        PostingEntry* first = getVectorItem(postings, 0);
        PostingEntry* second = getVectorItem(postings, 1);

        RUN_CHECK(first != NULL && first->doc_id == 1, "First posting doc_id is correct");
        RUN_CHECK(second != NULL && second->doc_id == 2, "Second posting doc_id is correct");
        RUN_CHECK(first != NULL && strcmp(first->title, "First title") == 0, "First posting title is correct");
        RUN_CHECK(second != NULL && strcmp(second->title, "Second title") == 0, "Second posting title is correct");
    }

    freeAVLTree(tree4);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Size: Counting only unique keys\n");
    AVLTree* tree5 = createAVLTree();

    avlInsert(tree5, "python", 1, "Python title");
    avlInsert(tree5, "java", 2, "Java title");
    avlInsert(tree5, "python", 3, "Another python title");
    avlInsert(tree5, "c", 4, "C title");

    RUN_CHECK(tree5->size == 3, "Tree size counts only unique keys");

    freeAVLTree(tree5);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Left rotation\n");
    AVLTree* tree6 = createAVLTree();

    avlInsert(tree6, "a", 1, "A");
    avlInsert(tree6, "b", 2, "B");
    avlInsert(tree6, "c", 3, "C");

    RUN_CHECK(tree6->root != NULL, "Root exists after rotation");
    RUN_CHECK(strcmp(tree6->root->key, "b") == 0, "Root became 'b'");
    RUN_CHECK(strcmp(tree6->root->left->key, "a") == 0, "Left child is 'a'");
    RUN_CHECK(strcmp(tree6->root->right->key, "c") == 0, "Right child is 'c'");

    freeAVLTree(tree6);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Right rotation\n");
    AVLTree* tree7 = createAVLTree();

    avlInsert(tree7, "c", 1, "C");
    avlInsert(tree7, "b", 2, "B");
    avlInsert(tree7, "a", 3, "A");

    RUN_CHECK(tree7->root != NULL, "Root exists after rotation");
    RUN_CHECK(strcmp(tree7->root->key, "b") == 0, "Root became 'b'");
    RUN_CHECK(strcmp(tree7->root->left->key, "a") == 0, "Left child is 'a'");
    RUN_CHECK(strcmp(tree7->root->right->key, "c") == 0, "Right child is 'c'");

    freeAVLTree(tree7);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Left-right rotation\n");
    AVLTree* tree8 = createAVLTree();

    avlInsert(tree8, "c", 1, "C");
    avlInsert(tree8, "a", 2, "A");
    avlInsert(tree8, "b", 3, "B");

    RUN_CHECK(tree8->root != NULL, "Root exists after rotation");
    RUN_CHECK(strcmp(tree8->root->key, "b") == 0, "Root became 'b'");
    RUN_CHECK(strcmp(tree8->root->left->key, "a") == 0, "Left child is 'a'");
    RUN_CHECK(strcmp(tree8->root->right->key, "c") == 0, "Right child is 'c'");

    freeAVLTree(tree8);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Right-left rotation\n");
    AVLTree* tree9 = createAVLTree();

    avlInsert(tree9, "a", 1, "A");
    avlInsert(tree9, "c", 2, "C");
    avlInsert(tree9, "b", 3, "B");

    RUN_CHECK(tree9->root != NULL, "Root exists after rotation");
    RUN_CHECK(strcmp(tree9->root->key, "b") == 0, "Root became 'b'");
    RUN_CHECK(strcmp(tree9->root->left->key, "a") == 0, "Left child is 'a'");
    RUN_CHECK(strcmp(tree9->root->right->key, "c") == 0, "Right child is 'c'");

    freeAVLTree(tree9);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Traversal: Sorted in-order walk (avlTraverse)\n");
    AVLTree* tree10 = createAVLTree();

    avlInsert(tree10, "delta", 1, "Delta");
    avlInsert(tree10, "alpha", 2, "Alpha");
    avlInsert(tree10, "charlie", 3, "Charlie");
    avlInsert(tree10, "bravo", 4, "Bravo");

    const char* expected[] = {"alpha", "bravo", "charlie", "delta"};
    TraverseCtx ctx = { .expected = expected, .index = 0, .ok = true };

    avlTraverse(tree10, visit_check_order, &ctx);

    RUN_CHECK(ctx.ok == true, "Keys were visited in sorted order");
    RUN_CHECK(ctx.index == 4, "All 4 nodes were visited");

    freeAVLTree(tree10);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Memory Management: Free empty tree\n");
    AVLTree* tree11 = createAVLTree();

    freeAVLTree(tree11);
    freeAVLTree(NULL);

    RUN_CHECK(true, "freeAVLTree handled empty tree and NULL without crash");
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