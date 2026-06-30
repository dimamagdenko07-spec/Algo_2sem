#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ItemList *createItemList(int capacity)
{ 
    /*
    Принимает на вход вместимость списка
    Возвращает сам список
    Выделяем память под список и под массив элементов
    Устанавливаем дефолтные значения
    */
    ItemList *list = (ItemList*)malloc(sizeof(ItemList));
    if (list == NULL){
        printf("Ошибка выделения памяти\n");
        return NULL;
    }
    list->items = (Item*)malloc(sizeof(Item) * capacity);
    if (list->items == NULL){
        printf("Ошибка выделения памяти\n");
        return NULL;
    }
    list->capacity = capacity;
    list->size = 0;
    return list;
}

int addItem(ItemList *list, int weight, int value, const char *name)
{
    /*Принимает на вход список, вес, цену и имя нового предмета
    Добавляет элемент в список
    Возвращает код работы программы*/
    ((list->items) + list->size)->weight = weight;
    ((list->items) + list->size)->value = value;
    strcpy(((list->items) + list->size)->name, name);
    list->size++;
    return 0;
}

void freeItemList(ItemList *list)
{
    /*Получает на вход список
    Освобождает всю память выделенную под него*/
    free(list->items);
    free(list);
}

void printItemList(ItemList *list)
{
    /*Получает на вход список элементов
    Форматирует предметы под красивый вывод
    Выводит на экран отформатированный список*/
    printf("Name Weight Value\n");
    for (int i = 0; i < list->size; i++){
        Item *item = (Item*)(list->items + i);
        printf("%s  %d  %d\n", item->name, item->weight, item->value);
    }
}

static int **allocateDPTable(int rows, int cols)
{
    /*Принимает на вход кол-во строк и колонок
    Выделяет под таблицу память
    Под каждый элемент-указатель выделяет память, на которую он ссылается
    Возвращает DP таблицу*/
    int **DPtable = (int**)malloc(sizeof(int*)*rows);
    for (int i = 0; i < rows; i++){
        DPtable[i] = (int *)malloc(cols*sizeof(int));
    }
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            DPtable[i][j] = 0;
        }
    }
    return DPtable;
}

static void freeDPTable(int **dp, int rows)
{
    /*Принимает на вход таблицу и кол-во строк
    Очищает всё, на что выделялась память в самой таблице*/
    for (int i = 0; i < rows; i++){
        free(dp[i]);
    }
    free(dp);
}

static int maxInt(int a, int b)
{
    /*Принимает на вход два числа
    Возвращает наибольшее число*/
    if (a > b){
        return a;
    } else {
        return b;
    }
}


void printDPTable(int **dp, int n, int capacity, ItemList *items)
{
    /*Принимает на вход dp-таблицу, кол-во элементов, вместимость рюкзака, и список предметов
    Форматирует таблицу под красивый вывод и выводит на экран*/
    for (int i = 0; i < capacity+1; i++){
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i <= n; i++){
        if (i == 0){
            printf("Empty ");
        } else {
            printf("%-10s ", (items->items + i - 1)->name);
        }
        for (int j = 0; j <= capacity; j++){
            printf("%-5d ", dp[i][j]);
        }
        printf("\n");
    }
}

void reconstructSolution(int **dp, ItemList *items, int capacity, int *taken)
{
    /*Получает на вход dp-таблицу, список предметов, вместимость рюкзака и список флагов взятых предметов
    Проходит по таблицы сзади вперёд и ищет, откуда вы пришли.
    После этого добавляет в taken флаг 1 для предмета, который мы взяли*/
    int i = items->size;
    int j = capacity;
    while (i >= 1 && j >= 0){
        if (dp[i][j] != dp[i-1][j]){
            taken[i-1] = 1;
            j -= (items->items + i - 1)->weight;
            i -= 1;
        } else {
            i -= 1;
        }
    }
}

KnapsackResult *knapsack01(ItemList *items, int capacity)
{
    /*Понимаем на вход структуру предметов и вместимость рюкзака
    Ищем таблицу dp, а после с помощью ей восстанавливаем решение
    Добавляем все характеристики ответа в структуру*/
    int **dp = allocateDPTable(items->size+1, capacity+1);
    for (int i = 1; i <= items->size; i++){
        int weight = (items->items + i - 1)->weight;
        int value = (items->items + i - 1)->value;
        for (int j = 1; j <= capacity; j++){
            if (j < weight) {
                dp[i][j] = dp[i-1][j];
            } else {
                dp[i][j] = maxInt(dp[i-1][j], dp[i-1][j - weight] + value);
            }
        }
    }
    KnapsackResult *res = (KnapsackResult *)malloc(sizeof(KnapsackResult));
    res->totalValue = 0;
    res->totalWeight = 0;
    res->taken = (int *)malloc(sizeof(int)*(items->size));
    for (int i = 0; i < items->size; i++){
        res->taken[i] = 0;
    }
    res->takenCount = 0;
    reconstructSolution(dp, items, capacity, res->taken);
    for (int i = 0; i < items->size; i++){
        if (res->taken[i] == 1){
            res->takenCount++;
            res->totalValue += (items->items + i)->value;
            res->totalWeight += (items->items + i)->weight;
        }
    }
    printDPTable(dp, items->size, capacity, items);
    freeDPTable(dp, items->size+1);
    return res;
}

int knapsack01Optimized(ItemList *items, int capacity)
{
    /*Получаем на вход массив структур предметов и вместимость рюкзака
    Решаем рюкзак с помощью одномерной таблицы и возвращаем результат*/
    int *dp = (int*)malloc(sizeof(int)*(capacity+1));
    for (int i = 0; i <= capacity; i++){
        dp[i] = 0;
    }
    for (int i = 1; i <= items->size; i++){
        int weight = (items->items + i - 1)->weight;
        for (int j = capacity; j >= weight; j--){
            dp[j] = maxInt(dp[j], dp[j - weight] + (items->items + i - 1)->value);
        }
    }
    int result = dp[capacity];
    free(dp);
    return result;
}

void printKnapsackResult(KnapsackResult *result, ItemList *items)
{
    /*Получаем на вход структуру с результатом рюкзака и списком предметов
    Форматируем результат для красивого вывода и выводим*/
    printf("Taken %d rubles with the weight of %d kg\nTaken: ", result->totalValue, result->totalWeight);
    int written = 0;
    int i = 0;
    while (written < result->takenCount){
        if (result->taken[i] == 1){
            printf("%s ", (items->items + i)->name);
            written++;
        }
        i++;
    }
}

void freeKnapsackResult(KnapsackResult *result)
{
    /*Получает на вход структуру результата
    Очищает всю память, выделенную в структуре*/
    free(result->taken);
    free(result);
}
