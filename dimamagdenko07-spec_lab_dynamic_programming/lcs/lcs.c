#include "lcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int maxInt(int a, int b)
{
    return (a > b) ? a : b;
}

static int **allocateTable(int rows, int cols)
{
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
    if (table != NULL){
        for (int i = 0; i < rows; i++){
            free(table[i]);
        }
    }
    free(table);
}


int lcsLength(const char *s1, const char *s2)
{
    int rows = strlen(s1)+1;
    int cols = strlen(s2)+1;
    int **dp = allocateTable(rows, cols);
    if (dp == NULL){
        return -1;
    }
    for (int i = 1; i < rows; i++){
        for (int j = 1; j < cols; j++){
            if (s1[i-1] == s2[j-1]){
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = maxInt(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    int result = dp[rows-1][cols-1];
    freeTable(dp, rows);
    return result;
}

LCSResult *longestCommonSubsequence(const char *s1, const char *s2)
{
    int rows = strlen(s1)+1;
    int cols = strlen(s2)+1;
    int **dp = allocateTable(rows, cols);
    if (dp == NULL){
        return NULL;
    }
    for (int i = 1; i < rows; i++){
        for (int j = 1; j < cols; j++){
            if (s1[i-1] == s2[j-1]){
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = maxInt(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    int result = dp[rows-1][cols-1];
    LCSResult *res = (LCSResult*)malloc(sizeof(LCSResult));
    if (res == NULL){
        printf("Memory error");
        freeTable(dp, rows);
        return NULL;
    }
    res->length = result;
    res->subsequence = (char *)malloc(sizeof(char)*(result+1));
    if (res->subsequence == NULL){
        free(res);
        freeTable(dp, rows);
        printf("Memory error");
        return NULL;
    }
    res->dpTable = dp;
    res->rows = rows;
    res->cols = cols;
    res->subsequence[res->length] = '\0';
    int toPut = res->length - 1;
    int i = rows-1;
    int j = cols-1;
    while (i > 0 && j > 0){
        if (s1[i-1] == s2[j-1]){
            res->subsequence[toPut] = s1[i-1];
            toPut--;
            i--;
            j--;
        } else if (dp[i-1][j] == maxInt(dp[i-1][j], dp[i][j-1])){
            i--;
        } else {
            j--;
        }
    }
    return res;
}

char *reconstructLCS(int **dp, const char *s1, const char *s2, int i, int j)
{
    if (i == 0 || j == 0) {
        char *str = (char *)malloc(1);
        str[0] = '\0';
        return str;
    }
    if (s1[i - 1] == s2[j - 1]) {
        char *sub = reconstructLCS(dp, s1, s2, i - 1, j - 1);
        int len = strlen(sub);
        sub = (char *)realloc(sub, len + 2);
        sub[len] = s1[i - 1];
        sub[len + 1] = '\0';
        return sub;
    }
    if (dp[i - 1][j] >= dp[i][j - 1]) {
        return reconstructLCS(dp, s1, s2, i - 1, j);
    } else {
        return reconstructLCS(dp, s1, s2, i, j - 1);
    }
}


void printLCSTable(LCSResult *result, const char *s1, const char *s2)
{
    printf("     ");
    int **dp = result->dpTable;
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

void highlightLCS(const char *s1, const char *s2, const char *lcs)
{
    int lcsId = 0;
    int len = strlen(lcs);
    int len1 = strlen(s1);
    for (int i = 0; i < len1; i++){
        int sign = 0;
        if (lcsId < len){
            if (lcs[lcsId] == s1[i]){
                sign = 1;
                lcsId++;
            }
        }
        if (sign == 1){
            printf("[%c]", s1[i]);
        } else {
            printf("%c", s1[i]);
        }
    }
    lcsId = 0;
    printf("\n");
    int len2 = strlen(s2);
    for (int j = 0; j < len2; j++){
        int sign = 0;
        if (lcsId < len){
            if (lcs[lcsId] == s2[j]){
                sign = 1;
                lcsId++;
            }
        }
        if (sign == 1){
            printf("[%c]", s2[j]);
        } else {
            printf("%c", s2[j]);
        }
    }
}


static int **computeLCSTableForLines(char **lines1, int count1, char **lines2, int count2)
{
    int rows = count1+1;
    int cols = count2+1;
    int **dp = allocateTable(rows, cols);
    if (dp == NULL){
        return NULL;
    }
    for (int i = 1; i < rows; i++){
        for (int j = 1; j < cols; j++){
            if (strcmp(lines1[i-1], lines2[j-1]) == 0){
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = maxInt(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    return dp;
}

DiffResult *diffLines(char **lines1, int count1, char **lines2, int count2)
{
    DiffResult *diff = (DiffResult *)malloc(sizeof(DiffResult));
    if (diff == NULL){
        printf("Memory error");
        return NULL;
    }
    int maxLen = 0;
    for (int i = 0; i < count1; i++){
        if (maxLen < strlen(lines1[i])){
            maxLen = strlen(lines1[i]);
        }
    }
    for (int i = 0; i < count1; i++){
        if (maxLen < strlen(lines2[i])){
            maxLen = strlen(lines2[i]);
        }
    }
    int **dp = computeLCSTableForLines(lines1, count1, lines2, count2);
    diff->count = dp[count1][count2];
    diff->lines = (DiffLine *)malloc(sizeof(DiffLine) * (count1+count2));
    for (int i = 0; i < count1+count2; i++){
        (diff->lines)[i].line = (char *)malloc(sizeof(char) * (maxLen + 1));
        if ((diff->lines)[i].line == NULL){
            for (int k = 0; k < i; k++){
                free((diff->lines)[k].line);
            }
            freeTable(dp, count1+1);
            free(diff->lines);
            free(diff);
            printf("Memory error");
            return NULL;
        }
    }
    int i = count1;
    int j = count2;
    int toPut = count1+count2-1;
    while (i > 0 || j > 0){
        if (i > 0 && j > 0 && strcmp(lines1[i-1], lines2[j-1]) == 0){
            diff->lines[toPut].type = ' ';
            memcpy(diff->lines[toPut].line, lines1[i-1], strlen(lines1[i-1]) + 1);
            i--;
            j--;
            toPut--;
        } else if (i > 0 && (j == 0 || dp[i-1][j] >= dp[i][j-1])){
            diff->lines[toPut].type = '-';
            memcpy(diff->lines[toPut].line, lines1[i-1], strlen(lines1[i-1])+1);
            i--;
            toPut--;
        } else {
            diff->lines[toPut].type = '+';
            memcpy(diff->lines[toPut].line, lines2[j-1], strlen(lines2[j-1])+1);
            j--;
            toPut--;
        }
    }

    int realCount = (count1 + count2) - (toPut + 1);
    for (int k = 0; k < realCount; k++) {
        diff->lines[k] = diff->lines[toPut + 1 + k];
    }
    diff->count = realCount;
    freeTable(dp, count1+1);
    return diff;
}


static char **readFileLines(const char *filename, int *count)
{
    if (filename == NULL || count == NULL){
        return NULL;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Openning error\n");
        return NULL;
    }
    char buffer[1024];
    int capacity = 10;
    char **lines = (char **)malloc(sizeof(char*)*capacity);
    if (lines == NULL){
        printf("Memory error\n");
        fclose(file);
        return NULL;
    }
    int putted = 0;
    while (fgets(buffer, 1024, file)){
        buffer[strcspn(buffer, "\n")] = '\0';
        if (putted >= capacity){
            capacity *= 2;
            char **tmp = (char **)realloc(lines, sizeof(char*)*capacity);
            if (tmp == NULL){
                for (int i = 0; i < putted; i++){
                    free(lines[i]);
                }
                printf("Memory error\n");
                fclose(file);
                return NULL;
            }
            lines = tmp;
        }
        lines[putted] = strdup(buffer);
        putted++;
    }
    memcpy(count, &putted, sizeof(int));
    return lines;
}

static void freeFileLines(char **lines, int count)
{
    for (int i = 0; i < count; i++){
        free(lines[i]);
    }
    free(lines);
}

DiffResult *diffFiles(const char *file1, const char *file2)
{
    int *count1 = (int*)malloc(sizeof(int));
    int *count2 = (int*)malloc(sizeof(int));
    if (count1 == NULL || count2 == NULL){
        printf("Memory error\n");
        return NULL;
    }
    char **lines1 = readFileLines(file1, count1);
    char **lines2 = readFileLines(file2, count2);
    if (lines1 == NULL || lines2 == NULL) {
        if (lines1) {
            freeFileLines(lines1, *count1);
        }
        if (lines2) {
            freeFileLines(lines2, *count2);
        }
        free(count1);
        free(count2);
        return NULL;
    }
    DiffResult *res = diffLines(lines1, *count1, lines2, *count2);
    freeFileLines(lines1, *count1);
    freeFileLines(lines2, *count2);
    free(count1);
    free(count2);
    return res;
}

void printDiff(DiffResult *result)
{
    if (result != NULL){
        printf("---------diff-----------\n");
        for (int i = 0; i < result->count; i++){
            printf("%c %s\n", result->lines[i].type, result->lines[i].line);
        }
    }
}


void freeLCSResult(LCSResult *result)
{
    freeTable(result->dpTable, result->rows);
    free(result->subsequence);
    free(result);
}

void freeDiffResult(DiffResult *result)
{
    for (int i = 0; i < result->count; i++){
        free(result->lines[i].line);
    }
    free(result->lines);
    free(result);
}
