#include "activity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ActivityList *createActivityList(int capacity)
{
    ActivityList *list = (ActivityList *)malloc(sizeof(ActivityList));
    if (list == NULL){
        printf("Memory error\n");
        return NULL;
    }

    list->items = (Activity*)malloc(sizeof(Activity)*capacity);
    if (list->items == NULL){
        free(list);
        printf("Memory error\n");
        return NULL;
    }
    list->size = 0;
    list->capacity = capacity;
    return list;
}

int addActivity(ActivityList *list, int start, int end, const char *name)
{
    if (list == NULL || name == NULL){
        return -1;
    }
    if (list->size >= list->capacity){
        return -1;
    }
    list->items[list->size].start = start;
    list->items[list->size].end = end;
    strcpy(list->items[list->size].name, name);
    list->size++;
    return 0;
}

void freeActivityList(ActivityList *list)
{
    free(list->items);
    free(list);
}

void printActivityList(ActivityList *list)
{
    printf("Party\tStart\tEnd\n");
    for (int i = 0; i < list->size; i++){
        printf("%s\t%d\t%d\n", list->items[i].name, list->items[i].start, list->items[i].end);
    }
}

static int compareByEndTime(const void *a, const void *b)
{
    if (a == NULL || b == NULL){
        return 67;
    }
    Activity act1 = *(Activity*)a;
    Activity act2 = *(Activity*)b;
    if (act1.end < act2.end){
        return -1;
    } else if ((act1.end > act2.end)) {
        return 1;
    } else {
        return 0;
    }
}

void swap(Activity *a, Activity *b){
    Activity temp = *a;
    *a = *b;
    *b = temp;
}

void sortByEndTime(ActivityList *list, int low, int high)
{
    if (low < high){
        int main_id = low + (high - low) / 2;
        int new_to_change = low;
        swap(list->items + main_id, list->items + high);
        for (int i = low; i < high; i++){
            if (compareByEndTime(&list->items[i], &list->items[high]) == -1) {
                swap(list->items + new_to_change, list->items + i);
                new_to_change++;
            }
        }
        swap(list->items + new_to_change, list->items + high);
        sortByEndTime(list, low, new_to_change-1);
        sortByEndTime(list, new_to_change+1, high);
    }
}

ActivityList *selectActivities(ActivityList *activities)
{
    if (activities == NULL || activities->size == 0){
        return NULL;
    }
    sortByEndTime(activities, 0, activities->size-1);
    ActivityList *res = createActivityList(activities->size);
    if (res == NULL){
        return NULL;
    }
    addActivity(res, activities->items[0].start, activities->items[0].end, activities->items[0].name);
    for (int i = 1; i < activities->size; i++){
        if (activities->items[i].start >= res->items[res->size-1].end){
            addActivity(res, activities->items[i].start, activities->items[i].end, activities->items[i].name);
        }
    }
    return res;
}

void printTimeline(ActivityList *all, ActivityList *selected)
{
    printf("Name\tStart\tEnd\n");
    for (int i = 0; i < selected->size; i++){
        printf("%s\t%d\t%d\n", selected->items[i].name, selected->items[i].start, selected->items[i].end);
    }
}
