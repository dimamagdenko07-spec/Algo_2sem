#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "extra_tasks.h"

int wordEquals(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b) == 0;
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
            void *popped = popVectorItem(vector, i);
            if (popped != NULL){
                free(popped);
            }
            i--;
        }
    }
    return 0;
}


void normalizeWord(char *word)
{
    /*Создаю переменную для хранения изменений*/
    char temp_str[strlen(word)+1];
    temp_str[0] = '\0';
    int index = 0;
    /*Прохожу по всем символам слова*/
    while (*(word+index) != '\0'){
        /*Если буква большая, то во временную переменную добавляем её маленькую*/
        if ((int)*(word+index) >= 65 && (int)*(word+index) <= 90){
            char up_to_low = (char)((int)*(word+index) + 32);
            int len = strlen(temp_str);
            temp_str[len] = up_to_low;
            temp_str[len+1] = '\0';
        } else 
        /*Если буква маленькая, добавляем её*/
        if ((int)*(word+index) >= 97 && (int)*(word+index) <= 122){
            int len = strlen(temp_str);
            temp_str[len] = *(word+index);
            temp_str[len+1] = '\0';
        }
        /*Другие символы не добавляются*/
        index++;
    }
    /*Очищаем поданое слово и вместо него вставляем очищенное: только из маленьких букв*/
    memset(word, 0, strlen(word)+1);
    memcpy(word, &temp_str, strlen(temp_str)+1);
}

Vector *textToBoW(const char *text)
{
    /*Некорректные данные*/
    if (text == NULL){
        return NULL;
    }
    /*Создаём вектор для хранения неочищенных слов*/
    Vector *dirt_strs = createVector(strlen(text)+1);
    if (dirt_strs == NULL){
        return NULL;
    }
    char my_word[strlen(text)+1];
    my_word[0] = '\0';
    int index = 0;
    /*Проходим по тексту до пробела и записываем всё в одну переменную: 
    Добавляем символ в конец переменной и после него ещё знак конца строки
    Как только встретился пробел, если строка не пустая, добавляем в вектор необр. строк
    После этого очищаем строку*/
    while (*(text + index) != '\0'){
        if ((int)*(text + index) == 32 && strlen(my_word) != 0){
            appendVectorItem(dirt_strs, my_word);
            memset(my_word, 0, sizeof(my_word));
        }
        int len = strlen(my_word);
        my_word[len] = *(text + index);
        my_word[len+1] = '\0';
        index++;
    }
    /*Если после прохода по тексту переменная не пустая, добавляем её в вектор необр. строк*/
    if (strlen(my_word) > 0){
        appendVectorItem(dirt_strs, my_word);
    }
    /*Проходим по каждому слову необр вектора и обрабатываем его, оставляя только мал. буквы*/
    for (size_t i = 0; i < (*dirt_strs).size; i++){
        normalizeWord((char*)(*dirt_strs).data + (*dirt_strs).elem_size * i);
    }
    /*Если в векторе изначально были строки без букв, то после очистки есть пустые строки
    Поэтому мы их удаляем*/
    for (size_t i = 0; i < (*dirt_strs).size; i++){
        if (wordEquals((void*)((char*)(*dirt_strs).data + (*dirt_strs).elem_size * i), "") == 1){
            popVectorItem(dirt_strs, i);
            i--;
        } else if (wordEquals((void*)((char*)(*dirt_strs).data + (*dirt_strs).elem_size * i), "\0") == 1){
            popVectorItem(dirt_strs, i);
            i--;
        }
    }
    /*Создаём вектор с частотой и уникальными строками*/
    Vector *freq = createVector(sizeof(int));
    if (freq == NULL){
        return NULL;
    }
    Vector *unique = createVector(strlen(text)+1);
    if (unique == NULL){
        return NULL;
    }
    /*Добавляем в вектор уникальных строк все слова, которые есть в тексте*/
    for (size_t i = 0; i < (*dirt_strs).size; i++){
        appendVectorItem(unique, (char*)(*dirt_strs).data + (*dirt_strs).elem_size * i);
    }
    /*Удаляем дубникаты из вектора уникальных строк. Теперь там реально уникальные*/
    removeDuplicatesVector(unique, wordEquals);
    /*Заполняем вектор частот на столько, сколько элементов в векторе уник. строк*/
    for (size_t i = 0; i < (*unique).size; i++){
        int val = 0;
        appendVectorItem(freq, &val);
    }
    /*Проходим по вектору всех слов. Когда встретили слово,
    Добавляем +1 к частоте, которая соответствует этому слову в векторе уник. слов*/
    for (size_t i = 0; i < (*dirt_strs).size; i++){
        char *word = ((char*)(*dirt_strs).data + (*dirt_strs).elem_size * i);
        size_t index_in_unique = findVectorItem(unique, word, wordEquals);
        size_t new_value = *(int*)((char*)(*freq).data + (*freq).elem_size * index_in_unique)+1;
        setVectorItem(freq, index_in_unique, &new_value);
    }
    /*Создаём вектор для хранения инфы о каждом слове: слово и частота*/
    Vector *vocabulary = createVector(sizeof(WordFrequency));
    /*Для каждого слова в векторе уник. слов создаём структуру для хранения инфы
    После этого добавляем в сооответственные места само слово и его частоту
    Далее добавляем в наш новый вектор полученную структуру
    Это и будет Bag-of-Words, который нам нужно было реализовать*/
    for (size_t i = 0; i < (*unique).size; i++){
        WordFrequency elem = {.count =  *(int*)getVectorItem(freq, i)};
        strcpy(elem.word, (char*)getVectorItem(unique, i));
        appendVectorItem(vocabulary, &elem);
    }
    /*Под все вектора была выделена память, сами её очищаем*/
    vectorFree(dirt_strs);
    vectorFree(freq);
    vectorFree(unique);
    return vocabulary;
}

void printBoW(Vector *vocabulary) // вспомогательная функция для вывода результата
{
    printf("Bag-of-Words:\n");
    for (size_t i = 0; i < vocabulary->size; i++)
    {
        WordFrequency *wf = getVectorItem(vocabulary, i);
        printf("%s -> %d\n", wf->word, wf->count);
    }
}