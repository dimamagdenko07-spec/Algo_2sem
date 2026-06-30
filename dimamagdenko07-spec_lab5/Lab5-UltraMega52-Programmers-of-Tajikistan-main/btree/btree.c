// Тут как хотите, главное, чтобы интерфейс был таким же, как в avl и rbtree, чтобы не менять код в main.c
// Ну и переиспользуйте код из предыдущих лабораторных, если он вам подходит.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "btree.h"

// Функция для создания нодок, на всякий случай
BTreeNode* createBTreeNode() {
    BTreeNode *node = (BTreeNode *)calloc(1, sizeof(BTreeNode));  // Через 1-calloc зануляем все массивы
    if (!node) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    node->is_leaf = 1;  // Инициализируем как лист
    
    return node;
}  

BTree* createBTree(void) {
    BTree *tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    tree->size = 0;

    BTreeNode *root = createBTreeNode();
    if (!root){
        free(tree);
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }
    tree->root = root;

    return tree;
}

// Вспомогательная функция для рекурсивного освобождения детей
void freeBTreeNode(BTreeNode *node){
    if (!node) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    if (!node->is_leaf){
        for (int i = 0; i <= node->n; i++){
            freeBTreeNode(node->children[i]);
        }  
    }
    
    for (int i = 0; i < node->n; i++){
        if (node->keys[i]) free(node->keys[i]);
        if (node->postings[i]) vectorFree(node->postings[i]);
    }

    free(node);
}

void freeBTree(BTree* tree) {
    if (!tree) return;

    if (tree->root) freeBTreeNode(tree->root);

    free(tree);
}

// Функция для разделения переполненной ноды
void createSecondNode(BTreeNode* parent, BTreeNode* node, int index){
    BTreeNode *new_node = createBTreeNode();
    new_node->is_leaf = node->is_leaf;
    new_node->n = BTREE_T - 1;  // Делим детей ровно по половине

    for (int i = 0; i < BTREE_T - 1; i++){  // Правую половину ключей\ссылок отправляем в новую вершину
        new_node->keys[i] = node->keys[i + BTREE_T];
        new_node->postings[i] = node->postings[i + BTREE_T];
    }

    if (!node->is_leaf){  // Если не лист, то детей тоже делим
        for (int i = 0; i < BTREE_T; i++){
            new_node->children[i] = node->children[i + BTREE_T];
        }
    }

    node->n = BTREE_T - 1; // Левую половину оставляем старой ноде

    for (int i = parent->n; i >= index + 1; i--){
        parent->children[i + 1] = parent->children[i];
    }

    for (int i = parent->n - 1; i > index - 1; i--){
        parent->keys[i + 1] = parent->keys[i];
        parent->postings[i + 1] = parent->postings[i];
    }

    parent->keys[index] = node->keys[BTREE_T - 1];
    parent->postings[index] = node->postings[BTREE_T - 1];
    parent->children[index + 1] = new_node;
    parent->n++;
}

//Для вставки нам необходимо рекурсивно обойти деревцо в поисках подходящего листа
void btreeNodeInsert(BTreeNode* node, const char* key, int doc_id, const char* title){
    /*
    Если попали не в лист - сравниваем ключи, пока не найдем нужного потомка и идем ниже.
    Попали - ищем подходящее место, сдвигаем ключи и постинги и вставляем туда инфу
    */
    if (!node->is_leaf){
        int i = node->n - 1;

        while (i >= 0 && strcmp(key, node->keys[i]) < 0) i--;

        i++;

        if (node->children[i]->n == BTREE_MAX_KEYS){
            createSecondNode(node, node->children[i], i);

            if (strcmp(key, node->keys[i]) > 0) i++;
        }

        btreeNodeInsert(node->children[i], key, doc_id, title);
    }
    else{
        int i = node->n - 1;

        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            node->keys[i + 1] = node->keys[i];
            node->postings[i + 1] = node->postings[i];

            i--;
        }
        i++;

        node->keys[i] = strdup(key);
        
        Vector *post = createPostingList();
        appendPosting(post, doc_id, title);

        node->postings[i] = post;
        node->n++;
    }
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    Vector *postingList = btreeSearch(tree, key);
    if (postingList){ // Если ключ уже в дереве просто вставляем
        appendPosting(postingList, doc_id, title);
        return;
    }

    if (tree->root->n == BTREE_MAX_KEYS){ // Если дерево забито - время для расширения в глубину
        BTreeNode *new_root = createBTreeNode();
        new_root->is_leaf = 0;
        
        BTreeNode *old_root = tree->root;
        new_root->children[0] = old_root;
        tree->root = new_root;

        createSecondNode(tree->root, old_root, 0);
    }

    btreeNodeInsert(tree->root, key, doc_id, title);

    tree->size++;
}

Vector* btreeNodeSearch(BTreeNode *node, const char* key){
    /*
    На каждом шаге, сравнивая ключи, находим потомков и спускаемся
    */
    if (!node) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    int i = 0;

    while (i < node->n && strcmp(key, node->keys[i]) > 0) i++;

    if (i < node->n && strcmp(node->keys[i], key) == 0) return node->postings[i];

    if (node->is_leaf) return NULL;

    return btreeNodeSearch(node->children[i], key);
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    if (!tree) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    return btreeNodeSearch(tree->root, key);
}

void btreeTraverseNode(BTreeNode *node,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx){
    
    if (!node) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    for (int i = 0; i < node->n; i++){
        if (!node->is_leaf) btreeTraverseNode(node->children[i], visit, ctx);

        visit(node->keys[i], node->postings[i], ctx);
    }

    // Детей всегда на 1 больше, чем ключей, поэтому про последнего не забываем
    if (!node->is_leaf) btreeTraverseNode(node->children[node->n], visit, ctx);
}

void btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx) {
    
    if (!tree || !tree->root || !visit) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }

    btreeTraverseNode(tree->root, visit, ctx);
}