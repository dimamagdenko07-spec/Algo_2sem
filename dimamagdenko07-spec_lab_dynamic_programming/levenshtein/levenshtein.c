#include "levenshtein.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int minInt(int a, int b)
{
    /*Принимает на вход числа a и b
    Возвращает минимальное*/
    return (a < b) ? a : b;
}

static int minOfThree(int a, int b, int c)
{
    /*Принимает на вход числа и возвращает минимальное из них*/
    return minInt(minInt(a, b), c);
}

static int **allocateTable(int rows, int cols)
{
    /*Принимаем на вход строки и столбцы
    Выделяем память под dp таблицу,
    Заполняем её для нахождения ответа
    Возвращаем эту таблицу*/
    int **dp = (int **)malloc(sizeof(int *)*rows);
    if (dp == NULL){
        printf("Error with memory");
        return NULL;
    }
    for (int i = 0; i < rows; i++){
        dp[i] = (int*)malloc(sizeof(int)*cols);
        if (dp[i] == NULL){
            printf("Error with memory");
            for (int k = 0; k < i; k++){
                free(dp[k]);
            }
            free(dp);
            return NULL;
        }
    }
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            dp[i][j] = 0;
        }
    }
    return dp;
}

static void freeTable(int **table, int rows)
{
    /*Получаем на вход таблицу и строки
    Очищаем выделенную под таблицу память*/
    if (table != NULL){
        for (int i = 0; i < rows; i++){
            free(table[i]);
        }
    }
    free(table);
}

// Для отладки — возвращает имя операции
const char *operationName(OperationType type)
/*это не моя функция, писать ничего не буду*/
{
    switch (type)
    {
        case OP_NONE:    return "NONE";
        case OP_INSERT:  return "INSERT";
        case OP_DELETE:  return "DELETE";
        case OP_REPLACE: return "REPLACE";
        default:         return "UNKNOWN";
    }
}


int levenshteinDistance(const char *s1, const char *s2)
{
    /*Принимаем на вход две строки,
    Строим по ним dp таблицу под нашу задачу
    Возвращает правое нижнее значение - результат*/
    int rows = strlen(s1)+1;
    int cols = strlen(s2)+1;
    int **dp = allocateTable(rows, cols);
    if (dp == NULL){
        return -1;
    }
    for (int i = 0; i < rows; i++){
        dp[i][0] = i;
    }
    for (int j = 0; j < cols; j++){
        dp[0][j] = j;
    }
    for (int i = 1; i < rows; i++){
        for (int j = 1; j < cols; j++){
            if (s1[i-1] == s2[j-1]){
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = minOfThree(dp[i-1][j-1]+1, dp[i-1][j]+1, dp[i][j-1]+1);
            }
        }
    }
    int result = dp[rows-1][cols-1];
    freeTable(dp, rows);
    return result;
}

EditResult *levenshteinWithOperations(const char *s1, const char *s2)
{
    /*Принимает на вход 2 строки и находит результат, как и в предыдущей функции.
    Проходит с конца в начало, и разбирает:
    Какие символы мы удалили, какие вставили, а какие не меняли
    Записывает все данные в структуру под ответ и возвращает её*/
    int rows = strlen(s1)+1;
    int cols = strlen(s2)+1;
    int **dp = allocateTable(rows, cols);
    EditResult *edit = (EditResult*)malloc(sizeof(EditResult));
    if (edit == NULL){
        printf("Error with memory");
        return NULL;
    }
    edit->operationCount = 0;
    edit->distance = 0;
    edit->operations = (EditOperation*)malloc(sizeof(EditOperation)*(rows+cols));
    if (edit->operations == NULL){
        printf("Error with memory");
        return NULL;
    }
    if (dp == NULL){
        return NULL;
    }
    for (int i = 0; i < rows; i++){
        dp[i][0] = i;
    }
    for (int j = 0; j < cols; j++){
        dp[0][j] = j;
    }
    for (int i = 1; i < rows; i++){
        for (int j = 1; j < cols; j++){
            if (s1[i-1] == s2[j-1]){
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = minOfThree(dp[i-1][j-1]+1, dp[i-1][j]+1, dp[i][j-1]+1);
            }
        }
    }
    int result = dp[rows-1][cols-1];
    edit->distance = result;
    int i = rows-1;
    int j = cols-1;
    while (i > 0 && j > 0){
        if (s1[i-1] == s2[j-1]){
            i--;
            j--;
        } else {
            if (dp[i][j] == dp[i-1][j-1]+1){
                (edit->operations + edit->operationCount)->type = OP_REPLACE;
                (edit->operations + edit->operationCount)->position = i;
                (edit->operations + edit->operationCount)->oldChar = s1[i-1];
                (edit->operations + edit->operationCount)->newChar = s2[j-1];
                edit->operationCount++;
                i--;
                j--;
            } else if(dp[i][j] == dp[i-1][j]+1){
                (edit->operations + edit->operationCount)->type = OP_DELETE;
                (edit->operations + edit->operationCount)->position = i-1;
                (edit->operations + edit->operationCount)->oldChar = s1[i-1];
                (edit->operations + edit->operationCount)->newChar = '\0';
                edit->operationCount++;
                i--;
            } else {
                (edit->operations + edit->operationCount)->type = OP_INSERT;
                (edit->operations + edit->operationCount)->position = i;
                (edit->operations + edit->operationCount)->oldChar = '\0';
                (edit->operations + edit->operationCount)->newChar = s2[j-1];
                edit->operationCount++;
                j--;
            }
        }
    }
    while (i > 0){
        (edit->operations + edit->operationCount)->type = OP_DELETE;
        (edit->operations + edit->operationCount)->position = i;
        (edit->operations + edit->operationCount)->oldChar = s1[i-1];
        (edit->operations + edit->operationCount)->newChar = '\0';
        edit->operationCount++;
        i--;
    }
    while (j > 0){
        (edit->operations + edit->operationCount)->type = OP_INSERT;
        (edit->operations + edit->operationCount)->position = i;
        (edit->operations + edit->operationCount)->oldChar = '\0';
        (edit->operations + edit->operationCount)->newChar = s2[j-1];
        edit->operationCount++;
        j--;
    }
    freeTable(dp, rows);
    return edit;
}


void printEditTable(int **dp, const char *s1, const char *s2)
{
    /*Получает на вход dp-таблицу, строку 1 и строку 2
    Форматирует таблицу и строки для красивого вывода
    Выводит результат*/
    printf("     ");
    int rows = strlen(s1)+1;
    int cols = strlen(s2)+1;
    for (int j = 0; j < cols; j++){
        if (j != 0){
            printf("%c ", s2[j-1]);
        } else {
            printf("'' ");
        }
    }
    printf("\n");
    for (int i = 0; i < rows; i++){
        if (i != 0){
            printf("%c  ", s1[i-1]);
        } else {
            printf("'' ");
        }
        for (int j = 0; j < cols; j++){
            printf("%2d ", dp[i][j]);
        }
        printf("\n");
    }
}

void printOperations(EditResult *result, const char *s1, const char *s2)
{
    /*Получает на вход структуру результата, строку1 и строку2
    Обрабатывает, что происходило на операциях и выводит на экран*/
    int inserts = 0;
    int dels = 0;
    for (int i = result->operationCount-1; i > -1; i--){
        EditOperation *oper = (result->operations + i);
        if (oper->type == OP_REPLACE){
            printf("Operation REPLACE of %c with the %c at position %d\n", oper->oldChar, oper->newChar, oper->position + inserts - dels);
        } else if (oper->type == OP_INSERT){
            printf("Operation INSERT with %c at position %d\n", oper->newChar, oper->position + 1 + inserts - dels);
            inserts++;
        } else{
            printf("Operation DELETE of %c at position %d\n", oper->oldChar, oper->position + 1 + inserts - dels);
            dels++;
        }
    }
}

void printTransformation(EditResult *result, const char *s1, const char *s2)
{
    /*(вот тут вообще ад и тупые строки)
    Принимает на вход структуру с результатом, строку1 и строку2
    Обрабатывает, преобразовывает строки при выводе операций
    На каждом этапе мы видим, как выглядела строка*/
    int len1 = strlen(s1);
    char str[strlen(s2)+strlen(s1)+1];
    for (int i = 0; i < len1; i++){
        str[i] = s1[i];
    }
    str[len1] = '\0';
    int inserts = 0;
    int dels = 0;
    for (int i = result->operationCount-1; i > -1; i--){
        EditOperation *oper = (result->operations + i);
        if (oper->type == OP_REPLACE){
            printf("STEP %d: %s -> ", result->operationCount-i, str);
            str[oper->position-1-dels+inserts] = oper->newChar;
            printf("%s\n", str);
            printf("Operation REPLACE of %c with the %c at position %d\n", oper->oldChar, oper->newChar, oper->position + inserts - dels);
        } else if (oper->type == OP_INSERT){
            printf("STEP %d: %s -> ", result->operationCount-i, str);
            for (int i = len1-1; i >= oper->position-dels+inserts; i--){
                str[i+1] = str[i];
            }
            str[len1+1] = '\0';
            str[oper->position-dels+inserts] = oper->newChar;
            printf("%s\n", str);
            printf("Operation INSERT with %c at position %d\n", oper->newChar, oper->position + 1 + inserts - dels);
            inserts++;
            len1++;
        } else{
            printf("STEP %d: %s -> ", result->operationCount-i, str);
            for (int i = oper->position-dels+inserts; i < len1; i++){
                str[i-1] = str[i];
            }
            str[len1-1] = '\0';
            printf("%s\n", str);
            printf("Operation DELETE of %c at position %d\n", oper->oldChar, oper->position + inserts - dels);
            dels++;
            len1--;
        }
    }
}

char **findSimilarWords(const char *word, char **dictionary, int dictSize,
                        int maxDistance, int *resultCount)
{
    /*на вход подаётся слово, список слов для сравнения, размер списка,
    Максимальное расстояние левенштейна, кол-во итоговых подходящих слов
    Проходит по каждому слову списка и вычисляет расстояние левенштейна.
    Если оно меньше maxDistance, то увеличиваем resultCount,
    Добавляем в список итоговых слов*/
    char **answer = (char **)malloc(sizeof(char*)*dictSize);
    if (answer == NULL){
        return NULL;
    }
    int sizeOfStr = 0;
    for (int i = 0; i < dictSize; i++){
        if (strlen(dictionary[i]) > sizeOfStr){
            sizeOfStr = strlen(dictionary[i]);
        }
    }
    sizeOfStr++;
    int amountCount = 0;
    for (int i = 0; i < dictSize; i++){
        int result = levenshteinDistance(dictionary[i], word);
        if (result <= maxDistance){
            answer[amountCount] = (char *)malloc(sizeof(char) * sizeOfStr);
            if (answer[amountCount] == NULL){
                for (int k = 0; k < amountCount; k++){
                    free(answer[k]);
                }
                free(answer);
                return NULL;
            }
            memcpy(answer[amountCount], dictionary[i], strlen(dictionary[i]));
            answer[amountCount][strlen(dictionary[i])] = '\0';
            amountCount++;
        }
    }
    *resultCount = amountCount;
    return answer;
}

void freeSimilarWords(char **words, int count)
{
    /*Принимает на вход список слов и кол-во их
    Очищает память, выделенную под что-то в словах*/
    for (int i = 0; i < count; i++){
        free(words[i]);
    }
    free(words);
}


void freeEditResult(EditResult *result)
{
    /*очищаем*/
    free(result->operations);
    free(result);
}