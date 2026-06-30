#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "generic.h"



int create_vector_test(){
    /*Тест 1*/

    /*Проверка вектора определённого типа*/
    Vector *vec = createVector(sizeof(int));
    assert((*vec).size == 0);
    assert((*vec).data == NULL);
    assert((*vec).elem_size == sizeof(int));
    /*Проверка вектора случайного размера*/
    vec = createVector(100);
    assert((*vec).elem_size == 100);

    /*Конец теста 1*/
    return 0;
}


int append_vector_test(){
    /*Тест 2*/
    Vector *vec = createVector(sizeof(int));
    int value = 23;
    /*Некорректные данные*/
    assert(appendVectorItem(vec, NULL) == 1);
    assert(appendVectorItem(NULL, &value) == 1);
    /*Добавление элемента в пустой вектор*/
    appendVectorItem(vec, &value);
    assert(*(int* )(*vec).data == 23);
    /*Добавление элемента в непустой вектор*/
    value = 123;
    appendVectorItem(vec, &value);
    assert(*((int* )(*vec).data+1) == 123);
    /*Полное заполнение вектора*/
    for (int i = 0; i < 3; i++){
        appendVectorItem(vec, &i);
    }
    assert(*((int* )(*vec).data+2) == 0);
    assert(*((int* )(*vec).data+3) == 1);
    assert(*((int* )(*vec).data+4) == 2);
    /*Добавление элемента в заполненный вектор*/
    appendVectorItem(vec, &value);
    assert(*((int* )(*vec).data+5) == 123);
    /*Добавление большого количества элементов*/
    Vector *vec_2 = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec_2, &i);
    }
    for (int i = 0; i < 12343123;i++){
        assert(*((int* )(*vec_2).data+i) == i);
    }
    /*Конец теста 2*/
    return 0;
}


int get_elem_test(){
    /*Тест 3*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec, &i);
    }
    /*Проверка корректности данных*/
    assert(getVectorItem(NULL, 5) == NULL);
    assert(getVectorItem(vec, -1) == NULL);
    assert(getVectorItem(vec, 123431230) == NULL);
    /*Проверка первого элемента*/
    assert(*(int*)getVectorItem(vec, 0) == 0);
    /*Проверка последнего элемента*/
    assert(*(int*)getVectorItem(vec, 12343122) == 12343122);
    /*Элемент в середине*/
    assert(*(int*)getVectorItem(vec, 4322) == 4322);
    /*Конец теста 3*/
    return 0;
}


int set_vector_test(){
    /*Тест 4*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec, &i);
    }
    int value = 52;
    /*Проверка корректности данных*/
    assert(setVectorItem(NULL, 5, &value) == -1);
    assert(setVectorItem(vec, -1, &value) == -1);
    assert(setVectorItem(vec, 123431230, &value) == -1);
    assert(setVectorItem(vec, 123431, NULL) == -1);
    /*Изменение в начале*/
    assert(setVectorItem(vec, 0, &value) == 0);
    assert(*(int *)(getVectorItem(vec, 0)) == 52);
    /*Изменение в середине*/
    assert(setVectorItem(vec, 1556, &value) == 0);
    assert(*(int *)(getVectorItem(vec, 1556)) == 52);
    /*Изменение в конце*/
    assert(setVectorItem(vec, 12343122, &value) == 0);
    assert(*(int *)(getVectorItem(vec, 12343122)) == 52);
    /*Проверка всех значений вектора*/
    for (int i = 0; i < 12343123; i++){
        if (i == 0 || i == 1556 || i == 12343122){
            assert(*(int *)(getVectorItem(vec, i)) == 52);
        } else {
            assert(*(int *)(getVectorItem(vec, i)) == i);
        }
    }
    /*Конец теста 4*/
    return 0;
}


int pop_vector_test(){
    /*Тест 5*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec, &i);
    }
    /*Проверка некорректных данных*/
    assert(popVectorItem(vec, -1) == NULL);
    assert(popVectorItem(vec, 12343123) == NULL);
    assert(popVectorItem(NULL, 123123) == NULL);
    /*Проверка удаления последнего элемента*/
    void *test_val = popVectorItem(vec, 12343122);
    assert(*(int*)(test_val) == 12343122);
    free(test_val);
    /*Проверка удаления первого элемента*/
    test_val = popVectorItem(vec, 0);
    assert(*(int*)(test_val) == 0);
    free(test_val);
    /*Проверка удаления элемента в середине*/
    test_val = popVectorItem(vec, 1234);
    assert(*(int*)(test_val) == 1235);
    free(test_val);
    /*Проверка удаления последнего элемента после сдвигов элементов*/
    test_val = popVectorItem(vec, 12343119);
    assert(*(int*)(test_val) == 12343121);
    free(test_val);
    /*Проверка корректности всех элементов вектора после сдвигов*/
    for (int i = 0; i < 12343123; i++){
        if (i < 1234){
            assert(*(int*)getVectorItem(vec, i) == i+1);
        } else if (i >= 1234 && i < 12343119){
            assert(*(int*)getVectorItem(vec, i) == i+2);
        } else {
            assert(getVectorItem(vec, i) == NULL);
        }
    }
    /*Конец теста 5*/
    return 0;
}


int find_vector_test(){
    /*Тест 6*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec, &i);
    }
    /*Проверка некорректных данных*/
    int value = 12343124;
    assert(findVectorItem(NULL, &value, intEquals) == -1);
    assert(findVectorItem(vec, NULL, intEquals) == -1);
    /*Проверка не существующих в векторе данных*/
    assert(findVectorItem(vec, &value, intEquals) == -1);
    /*Проверка первого значения*/
    value = 0;
    assert(findVectorItem(vec, &value, intEquals) == 0);
    /*Проверка серединного значения*/
    value = 123341;
    assert(findVectorItem(vec, &value, intEquals) == 123341);
    /*Последнее значение*/
    value = 12343122;
    assert(findVectorItem(vec, &value, intEquals) == 12343122);
    /*Конец теста 6*/
    return 0;
}


int free_vec_test(){
    /*Тест 7*/
    Vector *vec = createVector(sizeof(int));
    for (int i = 0; i < 12343123;i++){
        appendVectorItem(vec, &i);
    }
    /*Проверка некорректных данных*/
    assert(vectorFree(NULL) == 1);
    /*Проверка корректных данных*/
    assert(vectorFree(vec) == 0);
    return 0;
    /*Конец теста 7*/
}


int main(){
    create_vector_test();
    append_vector_test();
    get_elem_test();
    set_vector_test();
    pop_vector_test();
    find_vector_test();
    free_vec_test();
    return 0;
}