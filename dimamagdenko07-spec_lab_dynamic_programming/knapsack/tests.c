#include "knapsack.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "knapsack.h"

ItemList* generateRandomItems(int n) {
    /*Случайные предметы*/
    ItemList *list = createItemList(n);
    for (int i = 0; i < n; i++) {
        addItem(list, rand() % 50 + 1, rand() % 100 + 1, "Item");
    }
    return list;
}

void runTestCase(const char* testName, ItemList *list, int capacity, int expectedValue) {
    printf("TEST: %s\n", testName);
    printf("Knapsack capacity: %d\n", capacity);
    printf("Expected max value: %d\n", expectedValue);
    printf("\n");
    

    KnapsackResult *res2D = knapsack01(list, capacity);
    printf("2D Algorithm\n");
    printKnapsackResult(res2D, list);
    printf("\n");


    int val1D = knapsack01Optimized(list, capacity);
    printf("1D Algorithm\n");
    printf("Max value: %d\n", val1D);


    if (res2D->totalValue == expectedValue && val1D == expectedValue) {
        printf("\nSUCCESS\n");
    } else {
        printf("\nFAILED\n");
        printf("Expected: %d, Got: 2D = %d, 1D = %d\n", expectedValue, res2D->totalValue, val1D);
    }
    printf("\n\n");

    // Cleanup
    freeKnapsackResult(res2D);
    freeItemList(list);
}

void runExperiments() {
    int N[] = {10, 20, 50, 100};
    int W[] = {100, 1000, 10000};

    printf("N\tW\tTime(s)\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            ItemList *list = generateRandomItems(N[i]);
            
            clock_t start = clock();
            knapsack01Optimized(list, W[j]);
            clock_t end = clock();
            /*время замеряется по тикам, а clock_per_sec - кол-во тиков в секунде*/
            printf("%d\t%d\t%f\n", N[i], W[j], (double)(end - start) / CLOCKS_PER_SEC);
            
            freeItemList(list);
        }
    }
}

int main() {
    ItemList *list;

    list = createItemList(3);
    addItem(list, 6, 30, "Item_A");
    addItem(list, 5, 20, "Item_B");
    addItem(list, 5, 20, "Item_C");
    runTestCase("Test 1: ", list, 10, 40);


    list = createItemList(3);
    addItem(list, 10, 60, "Laptop");
    addItem(list, 20, 100, "Monitor");
    addItem(list, 30, 120, "PC");
    runTestCase("Test 2: ", list, 50, 220);
    

    //слишком дорогой предмет
    list = createItemList(2);
    addItem(list, 10, 1000, "Diamond");
    addItem(list, 4, 50, "Gold");
    runTestCase("Item too heavy", list, 5, 50);

    //всё влезает
    list = createItemList(3);
    addItem(list, 1, 10, "A");
    addItem(list, 2, 20, "B");
    addItem(list, 3, 30, "C");
    runTestCase("Everything fits", list, 100, 60);

    //нулевая вместимость
    list = createItemList(3);
    addItem(list, 5, 100, "A");
    addItem(list, 5, 100, "B");
    addItem(list, 5, 100, "C");
    runTestCase("Zero capacity", list, 0, 0);
    
    srand(time(NULL));
    runExperiments();

    return 0;
}