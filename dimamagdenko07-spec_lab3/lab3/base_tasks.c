#include "base_tasks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Student *findMaxStudent(GenericList *list)
{
    /*Некорректные данные*/
    if (list == NULL){
        return NULL;
    }
    float max_point = -1;
    Node *curr_elem = list->head;
    Student *ptr_of_best = NULL;
    /*Проходим по всему списку и проверяем средний балл каждого узла*/
    while (curr_elem != NULL){
        if ((*(Student*)(curr_elem->data)).avg > max_point){
            ptr_of_best = (Student*)(curr_elem->data);
            max_point = (*(Student*)(curr_elem->data)).avg;
        }
        curr_elem = curr_elem->next;
    }
    return ptr_of_best;
}

void *findMaxVector(Vector *vector, EqualsFunc cmp)
{
    /*Некорректные данные*/
    if (vector == NULL || (*vector).size == 0){
        return NULL;
    }
    /*Выделяем память под возвращаемое значение*/
    void *ptr = malloc((*vector).elem_size);
    if (ptr == NULL){
        printf("Memory error");
        exit(EXIT_FAILURE);
    }
    /*Перекопируем первые данные в выделенный адрес*/
    memcpy(ptr, (*vector).data, (*vector).elem_size);
    /*Проходим по всему вектору и проверяем данные с хранящимися в выделенной памяти*/
    for (int i = 1; i < (*vector).size; i++){
        void *curr_elem = (void *)((char*)(*vector).data + (*vector).elem_size * i);
        /*Если данное значение больше хранящегося, копируем данное туда*/
        if (cmp(curr_elem, ptr) == 1){
            memcpy(ptr, curr_elem, (*vector).elem_size);
        }
    }
    return ptr;
}

int removeDuplicatesList(GenericList *list, EqualsFunc cmp)
{
    if (list == NULL){
        return -1;
    }
    Node *curr_elem = list->head;
    long long counter = 0;
    while (curr_elem != NULL){
        /*Если индекс первого элемента с данным значением меньше индекса,
        Значит повторяется, поэтому удаляем*/
        Node *next_elem = curr_elem->next;
        if (findItem(list, curr_elem->data, cmp) < counter){
            popItem(list, counter);
            counter--;
        }
        counter++;
        curr_elem = next_elem;
    }
    return 0;
}

int removeDuplicatesVector(Vector *vector, EqualsFunc cmp)
{
    if (vector == NULL){
        return -1;
    }
    for (int i = 0; i < (*vector).size; i++){
        /*Вычисляем адрес элемента с данным индексом*/
        void *curr_elem = (void*)((char*)(*vector).data + i * (*vector).elem_size);
        /*Если индекс первого элемента с данным значением меньше индекса,
        Значит повторяется, поэтому удаляем*/
        if (findVectorItem(vector, curr_elem, cmp) < i){
            popVectorItem(vector, i);
            i--;
        }
    }
    return 0;
}