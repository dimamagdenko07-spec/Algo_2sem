#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


GenericList *createList(size_t elem_size)
{
    /*Некорректные данные*/
    if (elem_size <= 0){
        printf("Error with size of element");
        exit(EXIT_FAILURE);
    }
    /*Выделяем память и проверяем, выделилась или нет*/
    GenericList *list = (GenericList*) malloc(sizeof(GenericList));
    if (!list){
        printf("Error with memory");
        exit(EXIT_FAILURE);
    }
    /*Присваем начальные значения*/
    list->head = NULL;
    list->elem_size = elem_size;
    return list;
}

void appendItem(GenericList *list, void *data)
{
    /*Проверка корректности входных данных*/
    if (list == NULL || data == NULL){
        printf("Incorrect arguments of function");
        exit(EXIT_FAILURE);
    }
    /*
    Рассматриваем два случае: 
        1 - если список пустой
        2 - если в списке есть элементы
    */
    if (list->head == NULL){
        /*Выделяем на голову столько байт, сколько весит узел
        Приводим войд указатель malloc к указателю на узел*/
        list->head = (Node*)malloc(sizeof(Node));
        if (list->head == NULL){
            printf("Error with memory");
            exit(EXIT_FAILURE);
        }
        list->head->next = NULL; //следующий элемент за последним - NULL
        /*Выделяем на data столько байт, сколько весит каждый элемент списка*/
        list->head->data = malloc((*list).elem_size);
        if (list->head->data == NULL){
            printf("Error with memory");
            exit(EXIT_FAILURE);
        }
        /*Копируем elem_size байт из data в data узла*/
        memcpy(list->head->data, data, list->elem_size);
    } else{
        Node *first_elem = list->head;
        Node *curr_elem = first_elem;
        /*Проходимся по списку до последнего элемента*/
        while (curr_elem->next != NULL){
            curr_elem = curr_elem->next;
        }
        /*После последнего элемента на следующий выделяем память
        Приводим указатель malloc типа void к Node*/
        curr_elem->next = (Node*)malloc(sizeof(Node));
        if (!curr_elem->next){
            printf("Error with memory");
            exit(EXIT_FAILURE);
        }
        curr_elem = curr_elem->next;
        /*Выделяем на data elem_size байт*/
        curr_elem->data = malloc((*list).elem_size);
        if (curr_elem->data == NULL){
            printf("Error with memory");
            exit(EXIT_FAILURE);
        }
        /*Копируем elem_size байт из data в data узла*/
        memcpy(curr_elem->data, data, (*list).elem_size);
        curr_elem->next = NULL; //после последнего символа следующий - NULL
    }
}

int findItem(GenericList *list, void *value, EqualsFunc cmp)
{
    /*Проверка корректности входных данных*/
    if (list == NULL || value == NULL){
        printf("Incorrect arguments of function");
        exit(EXIT_FAILURE);
    }
    /*Проверка списка на полноту*/
    if (list->head == NULL){
        return -1;
    }
    Node *curr_elem = list->head;
    int index_of_curr_elem = 0;
    /*Проходим по всем элементам и сравниваем с требуемым значением*/
    while (curr_elem->next != NULL){
        if (cmp(curr_elem->data, value) == 1){
            return index_of_curr_elem;
        }
        index_of_curr_elem +=1;
        curr_elem = curr_elem->next;
    }
    /*В цикле были рассмотрены все элементы кроме последнего, проверяем его*/
    if (cmp(curr_elem->data, value) == 1){
            return index_of_curr_elem;
        }
    return -1;
}

void *getItem(GenericList *list, unsigned int index)
{
    /* Проверка на корректность входных данных и наличие элементов */
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    Node *curr_elem = list->head;
    unsigned int curr_index = 0;

    /* Проходим по списку до нужного индекса */
    while (curr_elem != NULL) {
        if (curr_index == index) {
            return curr_elem;
        }
        curr_elem = curr_elem->next;
        curr_index++;
    }

    /* Если индекс оказался за пределами списка */
    return NULL;
}

void *popItem(GenericList *list, size_t index)
{
    /*Проверка на корректность входных данных и полноту списка*/
    if (list == NULL || index < 0 || list->head == NULL){
        return NULL;
    }
    /*Выделяем память под копию удаленного элемента,
    Чтобы он существовал после завершения функции*/
    int *copy_of_popped = malloc((*list).elem_size);
    if (copy_of_popped == NULL){
        printf("Error with memory");
        exit(EXIT_FAILURE);
    }
    Node  *previous_elem = list->head;
    Node *curr_elem = list->head->next;
    /*Удаление первого элемента. Отличается, так как используется head, а не next*/
    if (index == 0){
        memcpy(copy_of_popped, list->head->data, (*list).elem_size);
        Node *new_head = list->head->next;
        free(list->head->data);
        free(list->head);
        list->head = new_head;
        return copy_of_popped;
    }
    /*Удаление элемента любого, кроме первого. Если находим элемент данного индекса,
    Тогда копируем данные из его адреса в адрес переменной для копии.
    Копируем elem_size байт*/
    int curr_index = 1;
    while (curr_elem != NULL){
        if (index == curr_index){
            memcpy(copy_of_popped, curr_elem->data, (*list).elem_size);
            previous_elem->next = curr_elem->next;
            free(curr_elem->data);
            free(curr_elem);
            return copy_of_popped;
        }
        previous_elem = previous_elem->next;
        curr_elem = curr_elem->next;
        curr_index +=1;
    }
    /*Если программа не завершилась, то индекс больше кол-ва элементов*/
    if (curr_index - 1 < index){
        free(copy_of_popped);
        return NULL;
    }
    free(copy_of_popped);
    return NULL;
}

void freeList(GenericList *list)
{
    /*Проверяем, что list != Null*/
    if (list && list->head){
        Node *curr_elem = list->head;
        /*Проходим по каждому элементу и освобождаем память,
        Выделенную под следующий элемент и значение.
        Также для безопасности присваиваем каждому освобождённому указателю NULL*/
        while (curr_elem != NULL){
            Node *next_elem = curr_elem->next;
            free(curr_elem->data);
            curr_elem->data = NULL;
            free(curr_elem);
            curr_elem = NULL;
            curr_elem = next_elem;
        }
        list->head = NULL;
        /*Для самого list освобождаем память,
        Выделенную под сам list*/
        free(list);
    }
}

unsigned int listLength(GenericList *list)
{
    /*Проверка на корректность списка и наличие элементов в нём*/
    if (list == NULL || list->head == NULL){
        return 0;
    }
    Node *curr_elem = list->head;
    unsigned int len = 0;
    //Проходим по каждому элементу, чтобы определить длину списка
    while (curr_elem != NULL){
        curr_elem = curr_elem->next;
        len++;
    }
    return len;
}