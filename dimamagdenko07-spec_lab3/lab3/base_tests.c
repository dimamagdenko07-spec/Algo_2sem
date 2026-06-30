#include "base_tasks.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int find_max_student_test(){
    /*Некорректные значения*/
    assert(findMaxStudent(NULL) == NULL);
    /*Основные тесты*/
    Student st1 = { .name = "Hamster", .avg = 5.0};
    Student st2 = { .name = "Tefteli", .avg = 4.0};
    Student st3 = { .name = "KT", .avg = 1.0};
    Student st4 = { .name = "Vasya", .avg = 4.1};
    GenericList *list = createList(sizeof(Student));
    appendItem(list, &st1); appendItem(list, &st2);
    appendItem(list, &st3); appendItem(list, &st4);
    /*Проверка, когда max - нулевой элемент*/
    assert((*findMaxStudent(list)).avg == st1.avg);
    /*Проверка, когда max - последний элемент*/
    Student st5 = { .name = "GOD", .avg = 100.0};
    appendItem(list, &st5);
    assert((*findMaxStudent(list)).avg == st5.avg);
    /*Проверка, когда max - элемент в середине*/
    Student st6 = { .name = "Noname", .avg = 0.0};
    appendItem(list, &st6);
    assert((*findMaxStudent(list)).avg == st5.avg);
    return 0;
}


int max_vector_test(){
    /*Некорректные данные*/
    Vector *vec = createVector(sizeof(int));
    assert(findMaxVector(NULL, intGreater) == NULL);
    assert(findMaxVector(vec, intGreater) == NULL);
    /*Основные тесты*/
    int val = 3;
    appendVectorItem(vec, &val);
    /*Max среди одного элемента*/
    void *result = findMaxVector(vec, intGreater);
    assert(*(int*)result == 3);
    free(result);
    /*Max на нулевом месте*/
    val = 1;
    appendVectorItem(vec, &val);
    result = findMaxVector(vec, intGreater);
    assert(*(int*)result == 3);
    free(result);
    /*Max на последнем месте*/
    val = 122;
    appendVectorItem(vec, &val);
    result = findMaxVector(vec, intGreater);
    assert(*(int*)result == 122);
    free(result);
    /*Мах в середине*/
    val = 120;
    appendVectorItem(vec, &val);
    result = findMaxVector(vec, intGreater);
    assert(*(int*)result == 122);
    free(result);
    return 0;
}


int remove_duplicates_list_test(){
    /*Некорректные данные*/
    assert(removeDuplicatesList(NULL, intEquals) == -1);
    /*Основные тесты*/
    GenericList *list = createList(sizeof(int));
    for (int i = 0; i < 10; i++){
        appendItem(list, &i);
        appendItem(list, &i);
        appendItem(list, &i);
    }
    /*Удаление из всевозможных мест*/
    assert(removeDuplicatesList(list, intEquals) == 0);
    for (int i = 0; i < 10; i++){
        assert(findItem(list, &i, intEquals) == i);
    }
    /*Список без дублекатов*/
    assert(removeDuplicatesList(list, intEquals) == 0);
    for (int i = 0; i < 10; i++){
        assert(findItem(list, &i, intEquals) == i);
    }
    freeList(list);
    /*Список с одним уникальным элементом*/
    list = createList(sizeof(char[10]));
    char text[10] = "text";
    for (int i = 0; i < 10000; i++){
        appendItem(list, &text);
    }
    assert(removeDuplicatesList(list, intEquals) == 0);
    assert(listLength(list) == 1);
    assert(stringEquals(list->head->data, &text) == 1);
    return 0;
}


int remove_duplicates_vector_test(){
    /*Некорректные данные*/
    assert(removeDuplicatesVector(NULL, intEquals) == -1);
    /*Основные тесты*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 10; i++){
        appendVectorItem(vec, &i);
        appendVectorItem(vec, &i);
        appendVectorItem(vec, &i);
    }
    /*Удаление из всевозможных мест*/
    assert(removeDuplicatesVector(vec, intEquals) == 0);
    for (int i = 0; i < 10; i++){
        assert(findVectorItem(vec, &i, intEquals) == i);
    }
    /*Список без дублекатов*/
    assert(removeDuplicatesVector(vec, intEquals) == 0);
    for (int i = 0; i < 10; i++){
        assert(findVectorItem(vec, &i, intEquals) == i);
    }
    vectorFree(vec);
    /*Список с одним уникальным элементом*/
    vec = createVector(sizeof(char[10]));
    char text[10] = "text";
    for (int i = 0; i < 10000; i++){
        appendVectorItem(vec, &text);
    }
    assert(removeDuplicatesVector(vec, intEquals) == 0);
    assert((*vec).size == 1);
    assert(stringEquals((*vec).data, &text) == 1);
    return 0;
}


int main(){
    find_max_student_test();
    max_vector_test();
    remove_duplicates_list_test();
    remove_duplicates_vector_test();
    return 0;
}