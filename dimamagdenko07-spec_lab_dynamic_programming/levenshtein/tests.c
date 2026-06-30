#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "levenshtein.h"

char *generateRandomString(int length) {
    char *str = (char *)malloc(length + 1);
    for (int i = 0; i < length; i++) {
        str[i] = 'A' + (rand() % 26);
    }
    str[length] = '\0';
    return str;
}

void runLevenshteinTimeTest(int length) {
    char *s1 = generateRandomString(length);
    char *s2 = generateRandomString(length);

    clock_t start = clock();
    int dist = levenshteinDistance(s1, s2);
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Length: %d\tTime: %f seconds\n", length, time_taken);

    free(s1);
    free(s2);
}

void runLevenshteinTest(const char* testName, const char* s1, const char* s2, int expectedDistance) {
    printf("TEST: %s\n", testName);
    printf("Comparing: '%s' -> '%s'\n", s1, s2);
    printf("Expected Distance: %d\n", expectedDistance);

    int dist = levenshteinDistance(s1, s2);
    printf("Simple Distance: %d\n", dist);

    EditResult *result = levenshteinWithOperations(s1, s2);
    if (result != NULL) {
        printf("Result Distance: %d\n\n", result->distance);

        printf("[List of Operations]\n");
        printOperations(result, s1, s2);
        printf("\n");

        printf("[Transformation Steps]\n");
        printTransformation(result, s1, s2);

        if (result->distance == expectedDistance && dist == expectedDistance) {
            printf("\nSUCCESS\n");
        } else {
            printf("\nERROR\n");
        }

        freeEditResult(result);
    } else {
        printf("Memory еrror\n");
    }
    printf("\n\n");
}


void runDictionaryTest(const char* word, char** dictionary, int dictSize, int maxDist) {
    printf("TEST: Dictionary Search\n");
    printf("Target word: '%s'\n", word);
    printf("Max allowed distance: %d\n", maxDist);
    printf("Dictionary words: ");
    for(int i = 0; i < dictSize; i++) {
        printf("'%s' ", dictionary[i]);
    }
    printf("\n\n");

    int resultCount = 0;

    char **results = findSimilarWords(word, dictionary, dictSize, maxDist, &resultCount);

    if (results != NULL || resultCount == 0) {
        printf("Found %d similar words:\n", resultCount);
        for (int i = 0; i < resultCount; i++) {
            printf("- %s\n", results[i]);
        }
        printf("\nSUCCESS\n");

        if (results != NULL) {
            freeSimilarWords(results, resultCount);
        }
    } else {
         printf("\nERROR\n");
    }
    printf("\n\n");
}

int main() {
    // ТЕСТ 1: Базовый случай
    runLevenshteinTest("Simple replacement", "KOT", "KIT", 1);


    // ТЕСТ 2: Вставка символов
    runLevenshteinTest("Insertion only", "CAT", "CAST", 1);


    // ТЕСТ 3: Удаление символов
    runLevenshteinTest("Deletion only", "APPLE", "APE", 2);


    // ТЕСТ 4: Все буквы разные
    runLevenshteinTest("Completely different", "ABC", "XYZ", 3);


    // ТЕСТ 5: Одна строка пустая
    runLevenshteinTest("Empty string to word", "", "TEST", 4);
    runLevenshteinTest("Word to empty string", "TEST", "", 4);


    // ТЕСТ 6: Сложное преобразование
    runLevenshteinTest("hard transformation", "SANDYAAS", "SATURDAYYYY", 7);


    // ТЕСТ 7: Ищем опечатки к слову hello
    char *dict[] = {"hallo", "hell", "world", "hero", "hella", "yellow"};
    int dictSize = 6;
    runDictionaryTest("hello", dict, dictSize, 2);

    // ТЕСТ времени
    srand(time(NULL));
    int lengths[] = {10, 100, 1000, 10000};

    printf("Levenshtein Distance Time Test:\n");
    printf("-----------------------------------\n");
    for (int i = 0; i < 4; i++) {
        runLevenshteinTimeTest(lengths[i]);
    }
    return 0;
}