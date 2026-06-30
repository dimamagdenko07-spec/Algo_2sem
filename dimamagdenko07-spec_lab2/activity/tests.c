#include <stdio.h>
#include <stdlib.h>
#include "activity.h"


void run_activity_test(const char* test_name, ActivityList* list) {
    printf("========================================\n");
    printf("TEST: %s\n", test_name);
    printf("----------------------------------------\n");
    
    printf("Input Activities:\n");
    printActivityList(list);

    ActivityList* selected = selectActivities(list);

    if (selected != NULL) {
        printf("\nSelected Activities (Greedy Result):\n");
        printTimeline(list, selected);
        printf("\nTotal activities selected: %d\n", selected->size);
        freeActivityList(selected);
    } else {
        printf("\nResult is NULL (Empty list or Error)\n");
    }
    
    freeActivityList(list);
    printf("========================================\n\n");
}

int main() {
    ActivityList* list;


    // ТЕСТ 1
    list = createActivityList(6);
    addActivity(list, 1, 4, "Task_A");
    addActivity(list, 3, 5, "Task_B");
    addActivity(list, 0, 6, "Task_C");
    addActivity(list, 5, 7, "Task_D");
    addActivity(list, 3, 9, "Task_E");
    addActivity(list, 5, 9, "Task_F");
    run_activity_test("Standard Mixed Activities", list);


    // ТЕСТ 2
    list = createActivityList(3);
    addActivity(list, 1, 10, "Long_Job");
    addActivity(list, 2, 5, "Mid_Job");
    addActivity(list, 3, 4, "Short_Job");
    run_activity_test("All Overlapping (Should pick Short_Job)", list);


    // ТЕСТ 3
    list = createActivityList(3);
    addActivity(list, 1, 2, "Morning_Coffee");
    addActivity(list, 2, 3, "Work_Session");
    addActivity(list, 3, 4, "Dinner");
    run_activity_test("Sequential Activities (No Overlap)", list);


    // ТЕСТ 4
    list = createActivityList(1);
    addActivity(list, 10, 20, "Single_Task");
    run_activity_test("Single Activity Case", list);


    // ТЕСТ 5
    list = createActivityList(3);
    addActivity(list, 5, 10, "Task_1");
    addActivity(list, 2, 10, "Task_2");
    addActivity(list, 8, 10, "Task_3");
    run_activity_test("Identical End Times", list);

    // ТЕСТ 6
    list = createActivityList(2);
    addActivity(list, 1, 5, "First");
    addActivity(list, 5, 10, "Second");
    run_activity_test("Touch-point Boundaries (Start == End)", list);

    return 0;
}