#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "knapsack.h"


void print_header(const char *name) {
    printf("\n==================================================\n");
    printf(" RUNNING TEST: %s\n", name);
    printf("==================================================\n");
}

void test_standard_fractional() {
    print_header("Standard Fractional Case");
    ItemList *list = createItemList(3);
    addItem(list, 10, 60, "Gold");
    addItem(list, 20, 100, "Silver");
    addItem(list, 30, 120, "Bronze");

    double capacity = 50;
    printf("Capacity: %.2f\n", capacity);

    KnapsackResult *res = fractionalKnapsack(list, capacity);
    printKnapsackResult(res);
    printf("Total Value: %.2f (Expected: 240.00)\n", res->totalValue);
    
    freeKnapsackResult(res);
    freeItemList(list);
}

void test_small_capacity() {
    print_header("Very Small Capacity");
    ItemList *list = createItemList(2);
    addItem(list, 10, 100, "Big_Object");
    
    double capacity = 1;
    KnapsackResult *res = fractionalKnapsack(list, capacity);
    
    printKnapsackResult(res);
    printf("Total Value: %.2f (Expected: 10.00)\n", res->totalValue);

    freeKnapsackResult(res);
    freeItemList(list);
}


void test_compare_01_vs_fractional() {
    print_header("0/1 Naive vs Fractional Comparison");
    ItemList *list = createItemList(2);
    addItem(list, 10, 60, "Item_A");
    addItem(list, 8, 40, "Item_B");

    double capacity = 15;
    printf("Capacity: %.2f\n", capacity);

    double val01 = knapsack01Naive(list, capacity);
    KnapsackResult *resFrac = fractionalKnapsack(list, capacity);

    printf("0/1 Naive Result: %.2f\n", val01);
    printf("Fractional Result: %.2f\n", resFrac->totalValue);
    
    if (resFrac->totalValue > val01) {
        printf("Verdict: Fractional is better because it can split items!\n");
    }

    freeKnapsackResult(resFrac);
    freeItemList(list);
}

void test_zero_capacity() {
    print_header("Zero Capacity Edge Case");
    ItemList *list = createItemList(1);
    addItem(list, 10, 100, "Item");

    KnapsackResult *res = fractionalKnapsack(list, 0);
    printf("Items taken: %d (Expected: 0)\n", res->size);
    printf("Total Value: %.2f\n", res->totalValue);

    freeKnapsackResult(res);
    freeItemList(list);
}


void test_identical_ratios() {
    print_header("Identical Value/Weight Ratios");
    ItemList *list = createItemList(3);
    addItem(list, 10, 100, "A");
    addItem(list, 10, 100, "B");
    addItem(list, 10, 100, "C");

    KnapsackResult *res = fractionalKnapsack(list, 25);
    printKnapsackResult(res);
    printf("Total Value: %.2f (Expected: 250.00)\n", res->totalValue);

    freeKnapsackResult(res);
    freeItemList(list);
}


void test_zero_weight_item() {
    print_header("Zero Weight Item Protection");
    ItemList *list = createItemList(2);
    addItem(list, 0, 100, "Ghost_Item");
    addItem(list, 10, 50, "Normal_Item");

    KnapsackResult *res = fractionalKnapsack(list, 10);

    if (res) {
        printf("Handled zero weight correctly. Value: %.2f\n", res->totalValue);
        freeKnapsackResult(res);
    }
    freeItemList(list);
}

int main() {
    printf("Starting Knapsack Algorithm Tests...\n");

    test_standard_fractional();
    test_small_capacity();
    test_compare_01_vs_fractional();
    test_zero_capacity();
    test_identical_ratios();
    test_zero_weight_item();

    printf("\nAll tests finished.\n");
    return 0;
}