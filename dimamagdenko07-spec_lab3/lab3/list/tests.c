#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "generic.h"
#include "../comparators.h"


int list_generate_test(){
    /*Тесты создания списка: Тест 1*/
    GenericList *first_test_list = createList(4);
    assert(first_test_list != NULL);
    assert((*first_test_list).elem_size == 4);
    free(first_test_list);
    first_test_list = createList(sizeof(char));
    assert((*first_test_list).elem_size == 1);
    return 0;
    /*Конец теста 1*/
}


int append_item_test(){
    /*Тесты функции appendItem: Тест 2*/
    GenericList *second_test_list = createList(sizeof(int));
    //Значение int
    int test_elem = 4;
    appendItem(second_test_list, &test_elem);
    assert(*(int *)(*(*second_test_list).head).data == 4);
    //Граничное значение int
    test_elem = 2147483647;
    appendItem(second_test_list, &test_elem);
    assert(*(int *)(*(*(*second_test_list).head).next).data == 2147483647);
    //Проверка добавления нескольких элементов
    second_test_list = createList(sizeof(int));
    for (int i = 200; i <= 20000; i++){
        appendItem(second_test_list, &i);
    }
    Node *first_elem_test = second_test_list->head;
    int check_num = 200;
    while (first_elem_test->next != NULL){
        assert(*(int *)(*first_elem_test).data == check_num);
        first_elem_test = first_elem_test->next;
        check_num++;
    }
    assert(*(int *)(*first_elem_test).data == check_num);
    return 0;
    /*Конец теста 2*/
}


int find_item_test(){
    /*Тесты функции findItem: Тест 3*/
    GenericList *list = createList(sizeof(int));
    for (int i = 0; i < 231;i++){
        appendItem(list, &i);
    }
    //Проверка значения на 0 месте
    int test_num = 0;
    assert(findItem(list, &test_num, intEquals) == 0);
    //Проверка значения в середине
    test_num = 115;
    assert(findItem(list, &test_num, intEquals) == 115);
    //Проверка значения в конце
    test_num = 230;
    assert(findItem(list, &test_num, intEquals) == 230);
    //Проверка несуществующего элемента
    test_num = 123241;
    assert(findItem(list, &test_num, intEquals) == -1);
    //Проверка пустого списка
    char test_char = 'a';
    list = createList(sizeof(char));
    assert(findItem(list, &test_num, charEquals) == -1);
    //Проверка для списка строк
    list = createList(20);
    char test_str[10] = "LinAl(";
    appendItem(list, &test_str);
    strcpy(test_str, "MatAn(");
    appendItem(list, &test_str);
    strcpy(test_str, "Algos(");
    appendItem(list, &test_str);
    strcpy(test_str, "Tired((");
    appendItem(list, &test_str);
    char checking_str[7] = "MatAn(";
    assert(findItem(list, &checking_str, stringEquals) == 1);
    return 0;
    /*Конец теста 3*/
}


int pop_item_test(){
    /*Тесты функции popItem: Тест 4*/
    GenericList *list = createList(sizeof(int));
    for (int i = 5; i < 8;i++){
        appendItem(list, &i);
        appendItem(list, &i);
    }
    //удаление из начала. Список: 5-5-6-6-7-7
    assert(*(int *)(popItem(list, 0)) == 5);
    //удаление из середины. Список: 5-6-6-7-7
    assert(*(int *)(popItem(list, 2)) == 6);
    //Удаление последнего элемента. Список: 5-6-7-7
    assert(*(int *)(popItem(list, 3)) == 7);
    //Удаление элемента с индексом > длины списка. Список: 5-6-7
    assert(!popItem(list, 3));
    //Отрицательный индекс
    assert(!popItem(list, -3));
    /*Проверка корректности оставшихся значений. Корректный список: 5-6-7*/
    Node *curr_elem = list->head;
    int counter = 5;
    while (curr_elem){
        assert(*(int *)(curr_elem->data) == counter);
        curr_elem = curr_elem->next;
        counter += 1;
    }
    /*Проверка того, что в списке реально 3 элемента.
    Вычитаем единицу, так как после последней проверки
    counter увеличился на 1*/
    assert(counter-1 == 7);
    //Проверка пустого списка
    GenericList *list_1 = createList(20);
    assert(!popItem(list_1, 0));
    return 0;
    /*Конец теста 4*/
}


int free_list_test(){
    /*Тесты функции freeList: Тест 5*/
    GenericList *list = createList(sizeof(int));
    for (int i = 0; i <= 100; i++){
        appendItem(list, &i);
    }
    freeList(list);
    /*Конец теста 5*/
}


int list_length_test(){
    /*Тесты функции listLength: Тест 6*/
    GenericList *list = createList(sizeof(int));
    /*Тест пустого списка*/
    assert(listLength(list) == 0);
    /*Тест одноэлементного списка*/
    int test_val = 0;
    appendItem(list, &test_val);
    assert(listLength(list) == 1);
    /*Тест обычного листа из 1 + 233 элемента*/
    for (int i = 1; i < 234; i++){
        appendItem(list, &i);
    }
    assert(listLength(list) == 234);
    /*Тест некорректных данных*/
    assert(listLength(NULL) == 0);
    
    /*Конец теста 6*/
}

int main(){
    list_generate_test();
    append_item_test();
    find_item_test();
    pop_item_test();
    free_list_test();
    list_length_test();
    return 0;
}