#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


ItemList *createItemList(int capacity)
{
    ItemList *list = (ItemList *)malloc(sizeof(ItemList));
    if (list == NULL){
        printf("Memory error");
        return NULL;
    }
    list->capacity = capacity;
    list->size = 0;
    list->items = (Item *)malloc(sizeof(Item)*capacity);
    if (list->items == NULL){
        free(list);
        printf("Memory error");
        return NULL;
    }
    return list;
}

int addItem(ItemList *list, double weight, double value, const char *name)
{
    if (list != NULL && name != NULL && list->size < list->capacity){
        list->size++;
        list->items[list->size-1].weight = weight;
        list->items[list->size-1].value = value;
        strcpy(list->items[list->size-1].name, name);
        return 0;
    } else {
        return -1;
    }
}

void freeItemList(ItemList *list)
{
    if (list != NULL){
        free(list->items);
        free(list);
    }
}

void printItemList(ItemList *list)
{
    if (list != NULL){
        printf("Name\tWeight\tValue\n");
        for (int i = 0; i < list->size; i++){
            printf("%s\t%g\t%g\n", list->items[i].name, list->items[i].weight, list->items[i].value);
        }
    } else {
        printf("Memory error\n");
    }
}


double getValuePerWeight(Item *item)
{
    if (item == NULL || item->weight == 0){
        return -1;
    }
    return item->value / item->weight;
}

static int compareByValuePerWeight(const void *a, const void *b)
{
    double v1 = getValuePerWeight((Item*)a);
    double v2 = getValuePerWeight((Item*)b);
    if (v1 > v2 + 0.000001){
        return -1;
    }
    if (v2 > v1 + 0.000001){
        return 1;
    }
    return 0;
}

void swap(Item *a, Item *b){
    Item temp = *a;
    *a = *b;
    *b = temp;
}

void sortByValuePerWeight(ItemList *list, int low, int high)
{
    if (low < high){
        int main_id = low + (high - low) / 2;
        swap(list->items + high, list->items + main_id);
        int to_change = low;
        for (int i = low; i < high; i++){
            if (compareByValuePerWeight(list->items + i, list->items + high) == -1){
                swap(list->items + to_change, list->items + i);
                to_change++;
            }
        }
        swap(list->items + to_change, list->items + high);
        sortByValuePerWeight(list, low, to_change-1);
        sortByValuePerWeight(list, to_change+1, high);
    }
}


KnapsackResult *fractionalKnapsack(ItemList *items, double capacity)
{
    sortByValuePerWeight(items, 0, items->size-1);
    KnapsackResult *res = (KnapsackResult *)malloc(sizeof(KnapsackResult));
    if (res == NULL){
        printf("Memory error");
        return NULL;
    }
    res->items = (TakenItem *)malloc(sizeof(TakenItem)*(items->size));
    if (res->items == NULL){
        free(res);
        printf("Memory error");
        return NULL;
    }
    res->totalWeight = 0;
    res->totalValue = 0;
    res->size = 0;
    for (int i = 0; i < items->size; i++){
        if (res->totalWeight + items->items[i].weight <= capacity){
            res->items[res->size].weight = items->items[i].weight;
            res->items[res->size].value = items->items[i].value;
            strcpy(res->items[res->size].name, items->items[i].name);
            res->items[res->size].fraction = 1;
            res->size++;
            res->totalWeight += items->items[i].weight;
            res->totalValue += items->items[i].value;
        } else if (res->totalWeight < capacity && items->items[i].weight > 0){
            double fraction = (capacity - res->totalWeight) / items->items[i].weight;
            res->items[res->size].weight = items->items[i].weight * fraction;
            res->items[res->size].value = items->items[i].value * fraction;
            strcpy(res->items[res->size].name, items->items[i].name);
            res->items[res->size].fraction = fraction;
            res->size++;
            res->totalWeight += items->items[i].weight * fraction;
            res->totalValue += items->items[i].value * fraction;
        }
    }
    return res;

}


static double knapsack01Recursive(Item *items, int n, double capacity, int idx)
{
    if (idx >= n || capacity <= 0){
        return 0;
    }
    if (items[idx].weight > capacity){
        return knapsack01Recursive(items, n, capacity, idx+1);
    }
    double take = items[idx].value + knapsack01Recursive(items, n, capacity - items[idx].weight, idx+1);
    double skip = knapsack01Recursive(items, n, capacity, idx+1);
    if (take > skip){
        return take;
    } else {
        return skip;
    }
}

double knapsack01Naive(ItemList *items, double capacity)
{
    return knapsack01Recursive(items->items, items->size, capacity, 0);
}


void printKnapsackResult(KnapsackResult *result)
{
    if (result == NULL || result->items == NULL){
        printf("Memory error\n");
    } else {
        printf("Name\tWeight\tValue\tFraction\n");
        for (int i = 0; i < result->size; i++){
            printf("%s\t%g\t%g\t%g\n", result->items[i].name, result->items[i].weight, result->items[i].value, result->items[i].fraction);
        }
    }
}

void freeKnapsackResult(KnapsackResult *result)
{
    if (result != NULL){
        free(result->items);
        free(result);
    }
}
