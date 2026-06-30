#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcs.h"

void createTestFile(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fputs(content, file);
        fclose(file);
    } else {
        printf("ERROR: Cannot create test file '%s'\n", filename);
    }
}


void testLCS() {
    printf("TEST 3: LCS\n");

    const char *s1 = "ABCBDAB";
    const char *s2 = "BDCABA";

    printf("Strings: '%s' and '%s'\n\n", s1, s2);

    LCSResult *res = longestCommonSubsequence(s1, s2);
    if (res != NULL) {
        printf("LCS Length: %d\n", res->length);
        printf("LCS String: '%s'\n\n", res->subsequence);

        printf("[DP Table Visualized]\n");
        printLCSTable(res, s1, s2);
        printf("\n");

        printf("[Highlighted Subsequence]\n");
        highlightLCS(s1, s2, res->subsequence);

        freeLCSResult(res);
        printf("\nLCS TEST PASSED\n\n");
    } else {
        printf("LCS TEST FAILED\n\n");
    }
}


void testDiffArrays() {
    printf("TEST 4: DIFF ALGORITHM\n");

    int count1 = 4;
    char *lines1[] = {"int main() {", "    int a = 5;", "    return 0;", "}"};
    
    int count2 = 4;
    char *lines2[] = {"int main() {", "    int a = 10;", "    return 0;", "}"};

    printf("\n\n");

    DiffResult *diff = diffLines(lines1, count1, lines2, count2);
    if (diff != NULL) {
        printDiff(diff);
        freeDiffResult(diff);
        printf("\nARRAY DIFF TEST PASSED\n\n");
    } else {
        printf("ARRAY DIFF TEST FAILED\n\n");
    }
}


void testDiffFiles() {
    printf("TEST 5: DIFF ALGORITHM\n");

    const char *file1 = "old_version.txt";
    const char *file2 = "new_version.txt";

    createTestFile(file1, "Line 1: Apples\nLine 2: Bananas\nLine 3: Cherries\nLine 4: Dates\n");
    createTestFile(file2, "Line 1: Apples\nLine 2: Blueberries\nLine 3: Cherries\nExtra Line!\nLine 4: Dates\n");

    printf("Reading and comparing '%s' and '%s'...\n\n", file1, file2);

    DiffResult *diff = diffFiles(file1, file2);
    if (diff != NULL) {
        printDiff(diff);
        freeDiffResult(diff);
        printf("\nFILE DIFF TEST PASSED\n\n");
    } else {
        printf("FILE DIFF TEST FAILED\n\n");
    }


    remove(file1);
    remove(file2);
}

int main() {
    printf("\nFull testing\n\n");
    
    testLCS();
    testDiffArrays();
    testDiffFiles();

    printf("All completed\n");
    return 0;
}