#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generic.h"
// Вспомогательная функция для изменения размера
static bool needToResize(Vector *vector, bool *increase)
{
    // TODO - имплементировать
}

// Определяем увеличивать размер или уменьшать
static int resize(Vector *vector, bool increase)
{
    // TODO - имплементировать
}

Vector *createVector(size_t elem_size)
{
    /*Некорректные данные*/
    if (elem_size == 0){
        return NULL;
    }
    /*Выделяем память и проверяем, выделилась или нет*/
    Vector *vec = (Vector*)malloc(sizeof(Vector));
    if (vec == NULL){
        printf("Error with memory");
        exit(EXIT_FAILURE);
    }
    /*Присваиваем начальные значения вектору*/
    (*vec).elem_size = elem_size;
    (*vec).size = 0;
    (*vec).capacity = MIN_SIZE;
    (*vec).data = NULL;
    return vec;
}

int appendVectorItem(Vector *vector, void *el)
{
    /*Некорректные данные*/
    if (el == NULL || vector == NULL){
        return 1;
    }
    if ((*vector).capacity <= (*vector).size){
        /*Случай: Вместимость вектора кончилась*/
        if ((*vector).data == NULL){
            /*Если вектор не указывает на область данных
            Выделяем память под данные, изменяем характеристики
            Копируем элемент в нулевое место*/
            (*vector).data = malloc((*vector).elem_size * MIN_SIZE);
            if ((*vector).data == NULL){
                printf("Error with memory");
                exit(EXIT_FAILURE);
            }
            (*vector).capacity = MIN_SIZE;
            memcpy((*vector).data, el, (*vector).elem_size);
            (*vector).size += 1;
        }else{
            /*Выделяем побольше памяти в новом месте и копируем все данные в него*/
            void *place = malloc((*vector).elem_size * (*vector).capacity * 2);
            if (place == NULL){
                printf("Error with memory");
                exit(EXIT_FAILURE);
            }
            memcpy(place, (*vector).data, (*vector).elem_size * (*vector).size);
            free((*vector).data);
            (*vector).data = place;
            (*vector).capacity *= 2;
            void *ptr_to_append = (*vector).data;
            /*После копирования остаётся добавить новые элемент в конец*/
            ptr_to_append = ptr_to_append + (*vector).elem_size * ((*vector).size);
            memcpy(ptr_to_append, el, (*vector).elem_size);
            (*vector).size += 1;
        }
    } else{
        if ((*vector).data == NULL){
            /*Если вектор не указывает на область данных
            Выделяем память под данные, изменяем характеристики
            Копируем элемент в нулевое место*/
            (*vector).data = malloc((*vector).elem_size * MIN_SIZE);
            if ((*vector).data == NULL){
                printf("Error with memory");
                exit(EXIT_FAILURE);
            }
            (*vector).capacity = MIN_SIZE;
            memcpy((*vector).data, el, (*vector).elem_size);
            (*vector).size += 1;
        } else{
            /*Случай: Вектор полностью не заполнен*/
            void *ptr_to_append = (*vector).data;
            /*Копируем элемент в место после последнего*/
            ptr_to_append = ptr_to_append + (*vector).elem_size * ((*vector).size);
            memcpy(ptr_to_append, el, (*vector).elem_size);
            (*vector).size += 1;
        }
    }
    return 0;
}

void *getVectorItem(Vector *vector, size_t index)
{
    /*Некорректные данные*/
    if (vector == NULL || index >= (*vector).size){
        return NULL;
    }
    /*Расчитываем адрес элемента на месте index'а и возвращаем его*/
    void *ptr = (*vector).data;
    ptr = (void*)((char *)ptr + (*vector).elem_size * index);
    return ptr;
}

int setVectorItem(Vector *vector, size_t index, void *value)
{
    /*Некорректные данные*/
    if (vector == NULL || index >= (*vector).size || value == NULL){
        return -1;
    }
    /*Расчитываем адрес элемента на месте index'а и копируем туда новое значение*/
    void *ptr = (*vector).data;
    ptr = (void*)((char *)ptr + (*vector).elem_size * index);
    memcpy(ptr, value, (*vector).elem_size);
    return 0;
}

void *popVectorItem(Vector *vector, size_t index)
{
    /*Некорректные данные*/
    if (vector == NULL || index >= (*vector).size){
        return NULL;
    }
    /*Выделяем память под копию удаленного элемента*/
    void *ptr_popped = malloc((*vector).elem_size);
    void *ptr_to_del = (*vector).data;
    /*Расчитываем адрес элемента под удаление
    и копируем его в отведённое место*/
    void *place_of_elem = (void *)((char *) ptr_to_del + (*vector).elem_size * index);
    memcpy(ptr_popped, place_of_elem, (*vector).elem_size);
    /*Все следующие элементы после удалённого сдвигаем*/
    for (size_t i = index+1; i < (*vector).size; i++){
        void *ptr_to_move = (void *)((char *) ptr_to_del + (*vector).elem_size * i);
        void *ptr_where_to_move = (void *)((char *) ptr_to_del + (*vector).elem_size * (i-1));
        memcpy(ptr_where_to_move, ptr_to_move, (*vector).elem_size);
    }
    /*Меняем размер вектора*/
    (*vector).size--;
    return ptr_popped;
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp)
{
    /*Некорректные данные*/
    if (vector == NULL || value == NULL){
        return -1;
    }
    /*Ищет индекс элемента с данным значением*/
    void *ptr_elem = (*vector).data;
    for (size_t i = 0; i < (*vector).size; i++){
        void *curr_elem = (void *)((char *)ptr_elem + i * (*vector).elem_size);
        if (cmp(curr_elem, value) == 1){
            return i;
        }
    }
    return -1;
}

int vectorFree(Vector *vector)
{
    /*Некорректные данные*/
    if (vector == NULL){
        return 1;
    }
    /*Очищает выделенную с помощью malloc память*/
    free((*vector).data);
    (*vector).data = NULL;
    free(vector);
    return 0;
}