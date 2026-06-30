#include "generic.h"
#include "generic.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>




int hash_int_tests(){
    /*Тестирования функции HashInt*/
    printf("The beginning of tests HashInt\n");
    printf("1. Nums under 0 and 0\n");
    for (int i = -12334; i <= 0; i++){
        // ещё остаток, так как может быть ноль -> остаток будет без него TABLE_MIN_SIZE
        int hash = HashInt((void *)&i);
        assert(((abs(i) % TABLE_MIN_SIZE)) % TABLE_MIN_SIZE == hash % TABLE_MIN_SIZE);
    }
    printf("2. Positive nums\n");
    for (int i = 1; i < 12313; i++){
        int hash = HashInt((void *)&i);
        assert(i % TABLE_MIN_SIZE == hash % TABLE_MIN_SIZE);
    }
    printf("The end of tests HashInt\n\n");
    return 0;
}


int hash_string_tests(){
    /*Тестирования функции HashString*/
    printf("The beginning of tests HashString\n");
    printf("1. Empty string: ");
    char str_[10] = "";
    assert(0 == HashString1((void *)&str_) % TABLE_MIN_SIZE);
    printf("Good\n");
    printf("2. One char elems -> hash is 1: ");
    char one_char[1] = "x";
    assert(1 == HashString1((void *)&one_char) % TABLE_MIN_SIZE);
    printf("Good\n");
    printf("3. Usual String: ");
    char kasik[10] = "Kisik";
    int hash = (int)(pow((int)kasik[0], 4) +  pow((int)kasik[1], 3) + pow((int)kasik[2], 2) + pow((int)kasik[3], 1) + pow((int)kasik[4], 0));
    assert(hash % TABLE_MIN_SIZE == HashString1((void *)&kasik) % TABLE_MIN_SIZE);
    printf("Good\n");
    printf("The end of tests HashString\n\n");
    return 0;
}


int hash_table_create_test(){
    /*Тесты корректности создания хэш таблицы*/
    printf("The beginning of tests createHashTable\n");
    HashTable *hash_table = createHashTable(sizeof(char)*10, sizeof(int));
    /*Проверка характеристик структуры*/
    assert(hash_table->capacity == TABLE_MIN_SIZE);
    assert(hash_table->key_size == sizeof(char)*10);
    assert(hash_table->val_size == sizeof(int));
    assert(hash_table->size == 0);
    assert(((Tuple *)hash_table->values->data)->flag == SLOT_EMPTY);
    printf("The end of tests createHashTable\n\n");
    return 0;
}


int set_table_item_test(){
    printf("The beginning of tests setItemHashTable\n");
    HashTable *hash_table = createHashTable(sizeof(int), sizeof(int));
    /*Добавление одного элемента в пустую хэш таблицу*/
    int key = 10, data = 5;
    setItemHashTable(hash_table, (void *)&key, (void*)&data, HashInt, intEquals);
    Vector *values = hash_table->values;
    Tuple *elem = (Tuple *)getVectorItem(values, key);
    assert(*(int*)elem->key == key);
    assert(*(int*)elem->value == data);
    assert(elem->flag == SLOT_OCCUPIED);
    assert(hash_table->size == 1);
    /*Изменения значения по ключу в хэш таблице*/
    data = 15;
    setItemHashTable(hash_table, (void *)&key, (void*)&data, HashInt, intEquals);
    values = hash_table->values;
    elem = (Tuple *)getVectorItem(values, key);
    assert(*(int*)elem->key == key);
    assert(*(int*)elem->value == data);
    assert(elem->flag == SLOT_OCCUPIED);
    assert(hash_table->size == 1);
    /*Добавление элементов с одинаковым хэшем*/
    key = 501;
    data = 45;
    setItemHashTable(hash_table, (void *)&key, (void*)&data, HashInt, intEquals);
    values = hash_table->values;
    elem = (Tuple *)getVectorItem(values, (key % hash_table->capacity)+1*1);
    assert(*(int*)elem->key == key);
    assert(*(int*)elem->value == data);
    assert(elem->flag == SLOT_OCCUPIED);
    assert(hash_table->size == 2);
}


int rehash_table_test(){
    /*Создаём таблицу, в которой больше TABLE_MIN_SIZE элементов, которая сама вызовет Rehash
    Затем ещё раз используем rehash для проверки функции для таблицы, которой не нужны изменения*/
    HashTable *hash_table = createHashTable(sizeof(int), sizeof(int));
    for (int i = 400; i < 800; i++){
        int val = 100;
        setItemHashTable(hash_table, &i, &val, HashInt, intEquals);
    }
    rehashHashTable(hash_table, HashInt, intEquals);
    for (int i = 400; i < 800; i++){
        Vector *values = hash_table->values;
        Tuple elem = *(Tuple *)getVectorItem(values, i);
        int elem_key = *(int*)elem.key;
        int elem_val = *(int *)elem.value;
        int flag = elem.flag;
        assert(elem_key == i);
        assert(elem_val == 100);
        assert(flag == SLOT_OCCUPIED);
    }
    
}


int get_item_test(){
    HashTable *hash_table = createHashTable(sizeof(int), sizeof(int));
    for (int i = 400; i < 800; i++){
        int val = 100+i;
        setItemHashTable(hash_table, &i, &val, HashInt, intEquals);
    }
    /*Проверка получение по несуществующему ключу и по реальному*/
    for (int i = 0; i < 1000; i++){
        if (i < 400 || i > 799){
            assert(getItemHashTable(hash_table, (void *)&i, HashInt, intEquals) == NULL);
        } else {
            assert(*(int*)getItemHashTable(hash_table, &i, HashInt, intEquals) == i+100);
        }
    }
}


int pop_item_test(){
    HashTable *hash_table = createHashTable(sizeof(int), sizeof(int));
    for (int i = 400; i < 800; i++){
        int val = 100+i;
        setItemHashTable(hash_table, &i, &val, HashInt, intEquals);
    }
    /*Проверка удаление по несуществующему ключу и по реальному*/
    for (int i = 0; i < 1000; i++){
        if (i < 400 || i > 799){
            assert(popItemHashTable(hash_table, (void *)&i, HashInt, intEquals) == NULL);
        } else {
            assert(*(int*)popItemHashTable(hash_table, &i, HashInt, intEquals) == i+100);
            assert(getItemHashTable(hash_table, &i, HashInt, intEquals) == NULL);
        }
    }
    assert(hash_table->size == 0);
}


int collisions(){
    /*Проверка когда все колизии и когда удаляется элемент не в колизии*/
    HashTable *hash_table = createHashTable(sizeof(int), sizeof(int));
    for (int i = 0; i < 100; i++){
        int key = (i*491);
        setItemHashTable(hash_table, (void *)&key, (void *)&i, HashInt, intEquals);
    }
    assert(getCollisionCount(hash_table, HashInt) == 99);
    int new_key = 0;
    popItemHashTable(hash_table, &new_key, HashInt, intEquals);
    assert(getCollisionCount(hash_table, HashInt) == 99);
}


int full_process(){
    HashTable *table = createHashTable(sizeof(int), sizeof(int));
    int key = 10, data = 1000;
    /*Поиск и удаление элемента в пустой таблице*/
    assert(getItemHashTable(table, &key, HashInt, intEquals) == NULL);
    assert(popItemHashTable(table, &key, HashInt, intEquals) == NULL);
    /*Вставка, получение и удаление одного элемента*/
    setItemHashTable(table, &key, &data, HashInt, intEquals);
    assert(*(int*)getItemHashTable(table, &key, HashInt, intEquals) == data);
    assert(*(int*)popItemHashTable(table, &key, HashInt, intEquals) == data);
    assert(getItemHashTable(table, &key, HashInt, intEquals) == NULL);
    /*Несколько ключей с одинаковым хэшем*/
    int key1 = 501, data1 = 10100;
    int key2 = 491*2+10, data2 = 101000;
    setItemHashTable(table, &key, &data, HashInt, intEquals);
    setItemHashTable(table, &key1, &data1, HashInt, intEquals);
    setItemHashTable(table, &key2, &data2, HashInt, intEquals);
    assert(*(int*)getItemHashTable(table, &key, HashInt, intEquals) == data);
    assert(*(int*)getItemHashTable(table, &key1, HashInt, intEquals) == data1);
    assert(*(int*)getItemHashTable(table, &key2, HashInt, intEquals) == data2);
    assert(*(int*)popItemHashTable(table, &key2, HashInt, intEquals) == data2);
    assert(*(int*)popItemHashTable(table, &key1, HashInt, intEquals) == data1);
    assert(*(int*)popItemHashTable(table, &key, HashInt, intEquals) == data);
    assert(table->size == 0);
    /*Переполнение таблицы*/
    for (int i = 0; i < 10000; i++){
        int val = i+10;
        setItemHashTable(table, &i, &val, HashInt, intEquals);
    }
    for (int i = 0; i < 10000; i++){
        assert(*(int*)getItemHashTable(table, &i, HashInt, intEquals) == i+10);
    }
    freeHashTable(table);
    /*Удаление и повторная вставка*/
    table = createHashTable(sizeof(char)*10, sizeof(int));
    char keystr[10] = "hello";
    data = 10;
    setItemHashTable(table, &keystr, &data, HashString, stringEquals);
    assert(*(int*)popItemHashTable(table, &keystr, HashString, stringEquals) == 10);
    setItemHashTable(table, &keystr, &data, HashString, stringEquals);
    Tuple *elem = getVectorItem(table->values, HashString(&keystr) % table->capacity);
    assert(stringEquals(elem->key, &keystr) == 1);
    assert(*(int*)elem->value == data);
    assert(elem->flag == SLOT_OCCUPIED);
    /*Некорректные входные данные*/
    setItemHashTable(NULL, &keystr, &data, HashString, stringEquals);
    setItemHashTable(table, &keystr, NULL, HashString, stringEquals);
    setItemHashTable(table, NULL, &data, HashString, stringEquals);
    rehashHashTable(NULL, HashInt, intEquals);
    assert(getItemHashTable(NULL, &keystr, HashString, stringEquals) == NULL);
    assert(getItemHashTable(table, NULL, HashString, stringEquals) == NULL);
    assert(popItemHashTable(NULL, &keystr, HashString, stringEquals) == NULL);
    assert(popItemHashTable(table, NULL, HashString, stringEquals) == NULL);
    assert(getCollisionCount(NULL, HashInt) == -1);
    freeHashTable(NULL);
}

int main()
{
    hash_int_tests();
    hash_string_tests();
    hash_table_create_test();
    set_table_item_test();
    rehash_table_test();
    get_item_test();
    pop_item_test();
    collisions();
    full_process();
    printf("1");
}
