#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "rbtree.h"

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
    } while (0)

typedef struct {
    char last_key[256];
    int count;
    bool is_sorted;
} TraverseStatus;

void check_order_visitor(const char* key, Vector* postings, void* ctx) {
    TraverseStatus* status = (TraverseStatus*)ctx;

    if (status->count > 0) {
        if (strcmp(status->last_key, key) >= 0) {
            status->is_sorted = false;
        }
    }

    if (postings == NULL) {
        status->is_sorted = false;
    }

    strncpy(status->last_key, key, 255);
    status->last_key[255] = '\0';

    status->count++;
}

int main() {
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Initialization: Creating empty Red-Black tree (createRBTree)\n");
    RBTree* tree1 = createRBTree();

    RUN_CHECK(tree1 != NULL, "RBTree structure allocated successfully");
    RUN_CHECK(tree1->size == 0, "Initial tree size is 0");
    RUN_CHECK(tree1->root == tree1->nil, "Root points to sentinel nil");
    RUN_CHECK(tree1->nil->color == RB_BLACK, "Sentinel nil node is black");

    freeRBTree(tree1);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Insertion and Search: Basic operations\n");
    RBTree* tree2 = createRBTree();

    rbInsert(tree2, "delta", 10, "Doc10");
    rbInsert(tree2, "alpha", 20, "Doc20");
    rbInsert(tree2, "zeta", 30, "Doc30");

    RUN_CHECK(tree2->size == 3, "Tree size after 3 inserts is 3");
    RUN_CHECK(rbSearch(tree2, "alpha") != NULL, "Existing key 'alpha' found");
    RUN_CHECK(rbSearch(tree2, "zeta") != NULL, "Existing key 'zeta' found");
    RUN_CHECK(rbSearch(tree2, "omega") == NULL, "Missing key 'omega' returned NULL");

    freeRBTree(tree2);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Duplicates: Adding postings to existing key\n");
    RBTree* tree3 = createRBTree();

    rbInsert(tree3, "apple", 1, "Title 1");
    rbInsert(tree3, "apple", 2, "Title 2");
    rbInsert(tree3, "apple", 3, "Title 3");

    Vector* apple_postings = rbSearch(tree3, "apple");

    RUN_CHECK(tree3->size == 1, "Duplicate keys do not increase unique key count");
    RUN_CHECK(apple_postings != NULL, "Posting list for duplicate key exists");
    RUN_CHECK(apple_postings != NULL && apple_postings->size == 3, "Posting list contains 3 entries");

    freeRBTree(tree3);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Ascending insert order\n");
    RBTree* tree4 = createRBTree();

    rbInsert(tree4, "a", 1, "T1");
    rbInsert(tree4, "b", 2, "T2");
    rbInsert(tree4, "c", 3, "T3");
    rbInsert(tree4, "d", 4, "T4");
    rbInsert(tree4, "e", 5, "T5");

    RUN_CHECK(tree4->size == 5, "Tree size is 5");
    RUN_CHECK(strcmp(tree4->root->key, "a") != 0, "Tree was rotated, root is no longer 'a'");
    RUN_CHECK(tree4->root->color == RB_BLACK, "Root is black after balancing");

    TraverseStatus status1 = {"", 0, true};

    rbTraverse(tree4, check_order_visitor, &status1);

    RUN_CHECK(status1.is_sorted == true, "Keys remain sorted after rotations");
    RUN_CHECK(status1.count == 5, "All 5 nodes were visited");

    freeRBTree(tree4);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Balancing: Complex zig-zag insert cases\n");
    RBTree* tree5 = createRBTree();

    rbInsert(tree5, "50", 1, "T");
    rbInsert(tree5, "20", 2, "T");
    rbInsert(tree5, "35", 3, "T");
    rbInsert(tree5, "80", 4, "T");
    rbInsert(tree5, "70", 5, "T");

    RUN_CHECK(tree5->size == 5, "Tree size is 5 after complex insert sequence");
    RUN_CHECK(tree5->root->color == RB_BLACK, "Root is black after complex balancing");

    TraverseStatus status2 = {"", 0, true};

    rbTraverse(tree5, check_order_visitor, &status2);

    RUN_CHECK(status2.is_sorted == true, "Tree remains a valid binary search tree");
    RUN_CHECK(status2.count == 5, "All 5 nodes were visited");

    freeRBTree(tree5);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Large Data: 200 insertions and search\n");
    RBTree* tree6 = createRBTree();

    char key[32];

    for (int i = 0; i < 200; i++) {
        sprintf(key, "word_%03d", i);
        rbInsert(tree6, key, i, "StabilityTest");
    }

    RUN_CHECK(tree6->size == 200, "Successfully inserted 200 unique keys");
    RUN_CHECK(rbSearch(tree6, "word_100") != NULL, "Search for 'word_100' successful");
    RUN_CHECK(rbSearch(tree6, "word_999") == NULL, "Search for missing key 'word_999' returned NULL");
    RUN_CHECK(tree6->root->color == RB_BLACK, "Root is black after many insertions");

    freeRBTree(tree6);
    printf("\n");

    printf(COLOR_YELLOW "[TEST]" COLOR_RESET " Memory Management: Free empty tree and NULL\n");
    RBTree* tree7 = createRBTree();

    freeRBTree(tree7);
    freeRBTree(NULL);

    RUN_CHECK(true, "freeRBTree handled empty tree and NULL without crash");
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