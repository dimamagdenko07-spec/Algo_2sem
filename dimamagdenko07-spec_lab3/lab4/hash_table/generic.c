#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../lab3/vector/generic.h"
#include "generic.h"
#include <math.h>


typedef struct Tuple{
    /*Кортеж, который хранит указатель на ключ, значение и флаг
    Флаги: E - empty, F - full, D - deleted*/
    void *key;
    void *value;
    int flag;
} Tuple;


int HashInt(const void *key)
{
    /*Принимает на вход войд указатель на место, где лежит ключ
    Возвращает захэшированное значение ключа*/
    int key_int = *(int*)key;
    /* Прибавляем модуль, чтобы значения были неотрицательные */
    int hash_key = abs(key_int);
    return hash_key;
}

int HashString1(const void *key)
{
    /*Принимает на вход войд указатель на первый начало строки
    Возвращает хэш строки, полученный записью в полиномиальном виде
    */
    char* curr_char = (char *)key;
    size_t key_len = 0;
    /*Высчитываем длину, проходя до знака завершения строки*/
    while (*curr_char != '\0'){
        key_len++;
        curr_char +=1;
    }
    curr_char = (char*)key;
    unsigned long long key_int = 0;
    /*Вычисляем хэш таким образом:
    Пример: hello -> int(h)**4 + int(e)**3 + int(l)**2 + int(l)**1 + int(o)*/
    for (int i = key_len-1; i > -1; i--){
        key_int += pow((int)(*(curr_char + key_len-1 -i)), i);
    }
    return (int)(key_int % INT_MAX);
}


int HashString(const void *key) {
    /*Известный алгоритм, взял его, чтобы разобраться*/
    //Указатель на первый символ
    const char *s = (const char *)key;
    unsigned long hash = 5381; //начальный хэш, выбранный случайно
    int c;
    /*Для каждого следующего символа сдвигаем хэш влево на 5 байт(умножаем на 32)*/
    while ((c = *s++))
        hash = ((hash << 5) + hash) + c;
    /*Приводим к INT*/
    return (int)(hash % INT_MAX);
}


HashTable *createHashTable(size_t key_size, size_t val_size)
{
    /*Создаём вектор значений, где хранятся кортежи*/
    Vector *hash_table_vec = createVector(sizeof(Tuple));
    /*Для каждого кортежа в векторе выделяем под него память, обозначаем атрибуты и добавляем*/
    for (int i = 0; i < TABLE_MIN_SIZE; i++){
        Tuple *tup = (Tuple*)malloc(sizeof(Tuple));
        tup->key = NULL;
        tup->value = NULL;
        tup->flag = SLOT_EMPTY;
        appendVectorItem(hash_table_vec, tup);
    }
    /*Устанавливаем вместимоть и выделяем память на таблицу*/
    hash_table_vec->capacity = TABLE_MIN_SIZE;
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (table == NULL){
        printf("Error with memory");
        return(NULL);
    }
    /*Устанавливаем атрибуты таблицы*/
    table->key_size = key_size;
    table->val_size = val_size;
    table->size = 0;
    table->capacity = TABLE_MIN_SIZE;
    table->values = hash_table_vec;
    return table;
}

void setItemHashTable(HashTable *table, void *key, void *data, HashFunc hash, CmpFunc cmp)
{
    /*Получает на вход указатели на таблицу, ключ и значение, а также функции хэширования и сравнения*/
    /*Выполняем операции, если корректные данные*/
    if (table != NULL && key != NULL && data != NULL){
        /*Хэшируем ключ и выделяем вектор значений*/
        int key_hash = hash(key) % table->capacity;
        Vector *main_vec = table->values;
        int setted = 0; //флаг: сделаны изменения или нет
        /*Проходим по всему вектору, и если находим значение с таким же ключом,
        То заменяем это значение. Если дошли до пустого, то такого нету*/
        for (int i = 0; i < table->capacity;i++){
            int proba = (key_hash + i*i) % table->capacity;
            Tuple *elem = (Tuple *)getVectorItem(main_vec, proba);
            if (elem->flag == SLOT_EMPTY){
                break;
            }
            if (elem->flag == SLOT_OCCUPIED){
                if (cmp(key, elem->key) == 1){
                    memcpy(elem->value, data, table->val_size);
                    setted = 1;
                    break;
                }
            }
        }
        /*Значение не найдено, поэтому ищем первую незаполненную ячейку
        Выделяем память под ключ, значение и перекопируем туда входные данные*/
        if (setted == 0){
            for (int i = 0; i < table->capacity; i++){
                int proba = (key_hash + i*i) % table->capacity;
                Tuple *elem = (Tuple *)getVectorItem(main_vec, proba);
                if (elem->flag != SLOT_OCCUPIED){
                    elem->key = malloc(table->key_size);
                    if (elem->key == NULL){
                        printf("memory error\n");
                        exit(EXIT_FAILURE);
                    }
                    elem->value = malloc(table->key_size);
                    if (elem->value == NULL){
                        printf("memory error\n");
                        exit(EXIT_FAILURE);
                    }
                    memcpy(elem->key, key, table->key_size);
                    memcpy(elem->value, data, table->val_size);
                    setted = 1;
                    elem->flag = SLOT_OCCUPIED;
                    table->size++;
                    break;
                }
            }
        }
        if (setted == 0 || table->size * 2 > table->capacity){
            rehashHashTable(table, hash, cmp);
            setItemHashTable(table, key, data, hash, cmp);
        }
    }
}

void rehashHashTable(HashTable *table, HashFunc hash, CmpFunc cmp)
{
    if (table != NULL){
        /*Создаем новый вектор по аналогии с createHashTable, только больше вместимость*/
        Vector *vec_of_past = table->values;
        int past_cap = table->capacity;
        Vector *hash_table_vec = createVector(sizeof(Tuple));
        int capacity = table->capacity * 2 + 1;
        for (int i = 0; i < capacity; i++){
            Tuple *tup = (Tuple*)malloc(sizeof(Tuple));
            tup->key = NULL;
            tup->value = NULL;
            tup->flag = SLOT_EMPTY;
            appendVectorItem(hash_table_vec, tup);
        }
        hash_table_vec->capacity = capacity;
        table->size = 0;
        table->capacity = capacity;
        table->values = hash_table_vec;


        /*Для каждого элемента из старого вектора добавляем его в новую хэш таблицу*/
        for (int i = 0; i < past_cap; i++){
            Tuple *tup = (Tuple*)getVectorItem(vec_of_past, i);
            if (tup->flag == SLOT_OCCUPIED){
                setItemHashTable(table, tup->key, tup->value, hash, cmp);
            }
        }
        /*Очищаем память под старый вектор значений*/
        vectorFree(vec_of_past);
    }
}

void *getItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL){
        return NULL;
    }
    /*Вычисляем хэш ключа*/
    Vector *main_vec = table->values;
    int index = hash(key) % table->capacity;
    /*Используем квадратичное пробирование. Если ключ есть, то он встретится по пути*/
    for (int i = 0; i < table->capacity; i++){
        int proba = (index + i*i) % table->capacity;
        /*Ищем элемент на пути и смотрим его атрибуты*/
        Tuple *elem = (Tuple *)getVectorItem(main_vec, proba);
        if (elem->key != NULL){
            if (elem->flag == SLOT_OCCUPIED){
                if (cmp(key, elem->key) == 1){
                    return elem->value;
                }
            }
        }
        if (elem->flag == SLOT_EMPTY){
            break;
        }
    }
    /*Элемента нет :(*/
    return NULL;
}

void *popItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL){
        return NULL;
    }
    if (getItemHashTable(table, key, hash, cmp) == NULL){
        return NULL;
    }
    /*Память под копию*/
    void *copy = malloc(table->val_size);
    if (copy == NULL){
        printf("Memory error\n");
        return NULL;
    }
    /*Аналогично с get, только если элемент встретился, перекопируем его значение в копию и делаем флаг удаления*/
    int index = hash(key) % table->capacity;
    Vector *main_vec = table->values;
    for (int i = 0; i < table->capacity; i++){
        int proba = (index + i*i) % table->capacity;
        Tuple *elem = (Tuple *)getVectorItem(main_vec, proba);
        if (elem->key != NULL){
            if (cmp(elem->key, key) == 1 && elem->flag == SLOT_OCCUPIED){
                elem->flag = SLOT_DELETED;
                memcpy(copy, elem->value, table->val_size);
                table->size--;
                return copy;
            }
        }
        if (elem->flag == SLOT_EMPTY){
            break;
        }
    }
    return NULL;
}

unsigned long int getCollisionCount(HashTable *table, HashFunc hash)
{
    if (table == NULL){
        return -1;
    }
    unsigned long int collisions = 0;
    Vector *main_vec = table->values;
    /*Идём по всем элементам и если хэш ключа элемента не равен индексу, то мы в коллизии*/
    for (int i = 0; i < table->capacity;i++){
        Tuple *elem = (Tuple *)getVectorItem(main_vec, i);
        if (elem->flag == SLOT_OCCUPIED){
            if (hash(elem->key) % table->capacity != i){
                collisions++;
            }
        }
    }
    
    return collisions;
}

void freeHashTable(HashTable *table)
{
    if (table != NULL){
        Vector *main_vec = table->values;
        for (int i = 0; i < table->capacity; i++){
            Tuple *elem = (Tuple *)getVectorItem(main_vec, i);
            /*Очищаем память везде, где её выделяли
            Не очищаем elem, так как он очистится в vectorFree*/
            if (elem != NULL){
                if (elem->key != NULL) free(elem->key);
                if (elem->value != NULL) free(elem->value);
            }
        }
        vectorFree(table->values);
        free(table);
    }
}