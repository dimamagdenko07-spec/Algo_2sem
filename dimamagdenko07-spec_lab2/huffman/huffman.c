#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_CHARS 256


MinHeap *createMinHeap(int capacity)
{
    MinHeap *heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (heap == NULL){
        printf("Memory error\n");
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    heap->nodes = (HuffmanNode**)malloc(sizeof(HuffmanNode*)*capacity);
    if (heap->nodes == NULL){
        free(heap);
        printf("Memory error\n");
        return NULL;
    }
    return heap;
}

static void swapNodes(HuffmanNode **a, HuffmanNode **b)
{
    HuffmanNode *temp = *a;
    *a = *b;
    *b = temp;
}

static void heapifyUp(MinHeap *heap, int idx)
{
    HuffmanNode **nodes = heap->nodes;
    while (idx > 0){
        int parent = (idx - 1) / 2;
        if (nodes[parent]->frequency > nodes[idx]->frequency){
            swapNodes(&nodes[parent], &nodes[idx]);
            idx = parent;
        } else {
            break;
        }
    }
}

static void heapifyDown(MinHeap *heap, int idx)
{
    HuffmanNode **nodes = heap->nodes;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    int smallest = idx;
    if (left < heap->size && nodes[smallest]->frequency > nodes[left]->frequency){
        smallest = left;
    }
    if (right < heap->size && nodes[smallest]->frequency > nodes[right]->frequency){
        smallest = right;
    }
    if (smallest != idx){
        swapNodes(&nodes[idx], &nodes[smallest]);
        heapifyDown(heap, smallest);
    }
}

void insertMinHeap(MinHeap *heap, HuffmanNode *node)
{
    if (heap->size < heap->capacity){
        heap->nodes[heap->size] = node;
        heapifyUp(heap, heap->size);
        heap->size++;
    }
}

HuffmanNode *extractMin(MinHeap *heap)
{
    HuffmanNode *min = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size-1];
    heap->size--;
    heapifyDown(heap, 0);
    return min;
}

void freeMinHeap(MinHeap *heap)
{
    if (heap != NULL){
        free(heap->nodes);
        free(heap);
    }
}

static HuffmanNode *createNode(char symbol, int frequency)
{
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    if (node == NULL){
        printf("Memory error\n");
        return NULL;
    }
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static int isLeaf(HuffmanNode *node)
{
    return node && !node->left && !node->right; // так и быть, оставил эту сложную функцию уже решённой
}


int *countFrequencies(const char *text, int *uniqueCount)
{
    if (text == NULL || uniqueCount == NULL){
        return NULL;
    }
    (*uniqueCount) = 0;
    int *freqs = (int *)malloc(sizeof(int) * 256);
    if (freqs == NULL){
        printf("Memory error\n");
        return NULL;
    }
    for (int i = 0; i < 256; i++){
        freqs[i] = 0;
    }
    int s = 0;
    while (text[s] != '\0'){
        unsigned char id = (unsigned char)text[s];
        if (freqs[id] == 0){
            (*uniqueCount)++;
        }
        freqs[id]++;
        s++;
    }
    return freqs;
}

HuffmanNode *buildHuffmanTree(const char *text)
{
    if (text == NULL || text[0] == '\0'){
        return NULL;
    }
    int uniqueCount = 0;
    int *freqs = countFrequencies(text, &uniqueCount);
    MinHeap *heap = createMinHeap(uniqueCount);
    for (int i = 0; i < 256; i++){
        if (freqs[i] != 0){
            HuffmanNode *node = createNode((char)i, freqs[i]);
            insertMinHeap(heap, node);
        }
    }
    while (heap->size > 1){
        HuffmanNode *min1 = extractMin(heap);
        HuffmanNode *min2 = extractMin(heap);
        HuffmanNode *parent = createNode('\0', min1->frequency + min2->frequency);
        parent->left = min2;
        parent->right = min1;
        insertMinHeap(heap, parent);
    }
    if (heap->size == 1) {
        HuffmanNode *onlyNode = extractMin(heap);
        HuffmanNode *root = createNode('\0', onlyNode->frequency);
        root->left = onlyNode;
        root->right = NULL;
        
        freeMinHeap(heap);
        free(freqs);
        return root;
    }
    HuffmanNode *root = extractMin(heap);
    freeMinHeap(heap);
    free(freqs);
    return root;
}

static void generateCodesRecursive(HuffmanNode *node, char *code, int depth, HuffmanCode *codes, int *count)
{
    if (node == NULL){
        return;
    }
    if (isLeaf(node)){
        codes[*count].symbol = node->symbol;
        strcpy(codes[*count].code, code);
        codes[*count].codeLength = depth;
        (*count)++;
    } else {
        code[depth] = '0';
        code[depth+1] = '\0';
        generateCodesRecursive(node->left, code, depth+1, codes, count);
        code[depth] = '1';
        generateCodesRecursive(node->right, code, depth+1, codes, count);
    }
}

HuffmanTable *generateCodes(HuffmanNode *root)
{
    char *code = (char *)malloc(sizeof(char) * 256);
    if (code == NULL){
        printf("Memory Error\n");
        return NULL;
    }
    code[0] = '\0';
    HuffmanCode *codes = (HuffmanCode *)malloc(sizeof(HuffmanCode) * 256);
    if (codes == NULL){
        free(code);
        printf("Memory Error\n");
        return NULL;
    }
    int *count = (int *)malloc(sizeof(int));
    if (count == NULL){
        free(code);
        free(codes);
        printf("Memory Error\n");
        return NULL;
    }
    *count = 0;
    generateCodesRecursive(root, code, 0, codes, count);
    HuffmanTable *table = (HuffmanTable*)malloc(sizeof(HuffmanTable));
    if (table == NULL){
        free(code);
        free(codes);
        free(count);
        printf("Memory Error\n");
        return NULL;
    }
    table->codes = codes;
    table->size = *count;
    table->root = root;
    free(code);
    free(count);
    return table;
}


static HuffmanCode *findCode(HuffmanTable *table, char symbol)
{
    if (table == NULL || table->codes == NULL){
        return NULL;
    }
    HuffmanCode *codes = table->codes;
    for (int i = 0; i < table->size; i++){
        if (codes[i].symbol == symbol){
            return &codes[i];
        }
    }
    return NULL;
}

char *encode(const char *text, HuffmanTable *table)
{
    if (text == NULL || text[0] == '\0' || table == NULL || table->codes == NULL){
        return NULL;
    }
    int capacity = 1;
    int s = 0;
    while (text[s] != '\0'){
        HuffmanCode *code = findCode(table, text[s]);
        if (code != NULL){
            capacity += code->codeLength;
        }
        s++;
    }
    char *encoded = (char *)malloc(sizeof(char) * capacity);
    if (encoded == NULL){
        printf("Memory Error\n");
        return NULL;
    }
    encoded[capacity-1] = '\0';
    s = 0;
    int size = 0;
    while (text[s] != '\0'){
        HuffmanCode *code = findCode(table, text[s]);
        if (code != NULL){
            int length = code->codeLength;
            for (int i = 0; i < length; i++){
                encoded[size] = code->code[i];
                size++;
            }
        }
        s++;
    }
    return encoded;
}

char *decode(const char *encoded, HuffmanNode *root)
{
    if (encoded == NULL || root == NULL){
        return NULL;
    }
    HuffmanNode *curr_node = root;
    int s = 0;
    int capacity = strlen(encoded)+1;
    char *text = (char *)malloc(sizeof(char) * capacity);
    int size = 0;
    while (encoded[s] != '\0'){
        if (encoded[s] == '0'){
                curr_node = curr_node->left;
            } else {
                curr_node = curr_node->right;
            }
        
        if (isLeaf(curr_node)){
            text[size] = curr_node->symbol;
            curr_node = root;
            size++;
        }
        s++;
    }
    text[size] = '\0';
    return text;
}

static void printTreeRecursive(HuffmanNode *node, char *prefix, int isLeft)
{
    if (node == NULL) {
        return;
    }

    printf("%s", prefix);
    printf(isLeft ? "|-- " : ">-- ");

    if (isLeaf(node)) {
        if (node->symbol == '\n') {
            printf("[\\n:%d]\n", node->frequency);
        } else if (node->symbol == ' ') {
            printf("[' ':%d]\n", node->frequency);
        } else {
            printf("[%c:%d]\n", node->symbol, node->frequency);
        }
    } else {
        printf("(%d)\n", node->frequency);
    }

    char newPrefix[512];
    sprintf(newPrefix, "%s%s", prefix, isLeft ? "|   " : "    ");

    printTreeRecursive(node->left, newPrefix, 1);
    printTreeRecursive(node->right, newPrefix, 0);
}

void printHuffmanTree(HuffmanNode *root)
{
    if (root == NULL) return;
    char prefix[512] = "";
    printTreeRecursive(root, prefix, 0);
}

void printHuffmanTable(HuffmanTable *table)
{
    if (table == NULL || table->codes == NULL) return;

    printf("\n--- Huffman Coding Table ---\n");
    printf("%-10s | %-15s | %-10s\n", "Symbol", "Code", "Length");
    printf("-------------------------------------------\n");

    for (int i = 0; i < table->size; i++) {
        char sym = table->codes[i].symbol;
        
        // Красивое отображение спецсимволов
        if (sym == '\n') {
            printf("%-10s | %-15s | %-10d\n", "\\n", table->codes[i].code, table->codes[i].codeLength);
        } else if (sym == ' ') {
            printf("%-10s | %-15s | %-10d\n", "' '", table->codes[i].code, table->codes[i].codeLength);
        } else if (sym == '\t') {
            printf("%-10s | %-15s | %-10d\n", "\\t", table->codes[i].code, table->codes[i].codeLength);
        } else {
            printf("%-10c | %-15s | %-10d\n", sym, table->codes[i].code, table->codes[i].codeLength);
        }
    }
    printf("-------------------------------------------\n");
}

CompressionStats getCompressionStats(const char *text, HuffmanTable *table)
{
    if (text == NULL || text[0] == '\0') return (CompressionStats){0, 0, 0.0, 0.0};
    CompressionStats stats;
    int len = strlen(text);
    stats.originalBits = strlen(text) * 8;
    stats.compressedBits = 0;
    stats.entropy = 0;
    int count = 0;
    int *freqs = countFrequencies(text, &count);
    for (int i = 0; i < table->size; i++){
        stats.compressedBits += table->codes[i].codeLength * freqs[(unsigned char)table->codes[i].symbol];
        double p = (double)freqs[(unsigned char)table->codes[i].symbol] / len;
        if (p != 0){
            stats.entropy -= p * (log(p) / log(2));
        }
    }
    stats.compressionRatio = (stats.compressedBits / (double)stats.originalBits) * 100;
    free(freqs);
    return stats;
}

void printCompressionStats(CompressionStats *stats)
{
    if (stats == NULL) return;

    printf("\n========= Compression Statistics =========\n");
    printf("%-20s : %d bits\n", "Original size", stats->originalBits);
    printf("%-20s : %d bits\n", "Compressed size", stats->compressedBits);
    printf("%-20s : %.2f%%\n", "Compression ratio", stats->compressionRatio);
    printf("%-20s : %.4f bits/symbol\n", "Shannon entropy", stats->entropy);
    
    double saved = 100.0 - stats->compressionRatio;
    printf("%-20s : %.2f%%\n", "Space saved", saved > 0 ? saved : 0);
    printf("==========================================\n");
}

void freeHuffmanTree(HuffmanNode *root)
{
    if (root == NULL) return;

    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);

    free(root);
}

void freeHuffmanTable(HuffmanTable *table)
{
    if (table == NULL) return;

    if (table->codes != NULL) {
        free(table->codes);
    }

    if (table->root != NULL) {
        freeHuffmanTree(table->root);
    }

    free(table);
}
