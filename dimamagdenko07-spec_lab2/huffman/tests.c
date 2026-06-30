#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

void run_huffman_test(const char* test_name, const char* text) {
    printf("\n" "========================================================\n");
    printf(" TEST: %s\n", test_name);
    printf("========================================================\n");
    printf("Original Text: \"%s\"\n\n", text);

    if (text == NULL || strlen(text) == 0) {
        printf("Skipping: Text is empty or NULL.\n");
        return;
    }

    HuffmanNode *root = buildHuffmanTree(text);
    if (!root) {
        printf("Error: Could not build Huffman Tree.\n");
        return;
    }

    HuffmanTable *table = generateCodes(root);
    if (!table) {
        printf("Error: Could not generate Huffman Table.\n");
        freeHuffmanTree(root);
        return;
    }

    printf("[Huffman Tree Visualized]\n");
    printHuffmanTree(root);
    
    printHuffmanTable(table);

    char *encoded = encode(text, table);
    if (encoded) {
        printf("\nEncoded string (bits):\n%s\n", encoded);

        char *decoded = decode(encoded, root);
        if (decoded) {
            printf("\nDecoded string:\n%s\n", decoded);

            if (strcmp(text, decoded) == 0) {
                printf("\n>>> VERDICT: SUCCESS (Original matches Decoded) <<<\n");
            } else {
                printf("\n!!! VERDICT: FAILURE (Data mismatch!) !!!\n");
            }
            free(decoded);
        }
        free(encoded);
    }

    CompressionStats stats = getCompressionStats(text, table);
    printCompressionStats(&stats);

    freeHuffmanTable(table);
}

int main() {
    // ТЕСТ 1
    run_huffman_test("Classic Textbook Example", "ABRACADABRA");

    // ТЕСТ 2
    run_huffman_test("Short Word", "HELLO");

    // ТЕСТ 3
    run_huffman_test("Spaces and Case Sensitivity", "Huffman Coding is Cool");

    // ТЕСТ 4
    run_huffman_test("Single Character Repeating", "AAAAAAA");

    // ТЕСТ 5
    run_huffman_test("Newlines and Tabs", "Line One\n\tLine Two\nDone.");

    // ТЕСТ 6
    run_huffman_test("English Pangram", "the quick brown fox jumps over the lazy dog");

    // ТЕСТ 7
    run_huffman_test("Numbers and Punctuation", "1, 2, 3... 10! (Zero is 0)");

    printf("\n" "ALL TESTS COMPLETED.\n");

    return 0;
}