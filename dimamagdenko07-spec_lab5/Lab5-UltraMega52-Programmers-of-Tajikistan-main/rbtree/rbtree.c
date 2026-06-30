// Полностью на ваше усмотрение (только переиспользуйте код из предыдущих лабораторных, если он вам подходит)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rbtree.h"


RBTree* createRBTree(void) {
    /*Ничего не принимает на вход
    Возвращает указатель на пустое красно-черное дерево*/
    RBTree *tree = (RBTree *)malloc(sizeof(RBTree));
    if (tree == NULL) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    tree->nil = (RBNode *)malloc(sizeof(RBNode));
    if (tree->nil == NULL) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        free(tree);
        return NULL;
    }

    /*Заполняем все необходимые поля*/
    tree->root = tree->nil;
    tree->nil->key = NULL;
    tree->nil->color = RB_BLACK;
    tree->nil->postings = NULL;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;
    tree->size = 0;
    return tree;
}

void freeNodes(RBNode* node, RBNode* nil) {
    /*Рекурсивная функция очистки узлов:
        Принимает на вход узел и sentinel-узел
        Очищает узел и переходит к его детям
        Ничего не возвращает*/
    if (node->left != nil) {
        freeNodes(node->left, nil);
    }

    if (node->right != nil) {
        freeNodes(node->right, nil);
    }

    free(node->key);
    vectorFree(node->postings);
    free(node);
}

void freeRBTree(RBTree* tree) {
    /*Принимает на вход дерево
    Рекурсивное функцией freeNodes очищает все узлы
    Очищает оставшиеся атрибуты
    Ничего не возвращает*/
    if (tree == NULL) {
        return;
    }

    if (tree->root != tree->nil) {
        freeNodes(tree->root, tree->nil);
    }

    free(tree->nil);
    free(tree);
}

void balancing(RBNode *node, RBTree *tree, RBNode *nil) {
    /*Вот это ПОЛНЫЙ п*здец. В этой функции деды переворачиваются, а дяди чернеют.
    Принимает на вход указатели на узел, дерево и sentinel-узел.
    Ничего не возвращает.
    Для удобства подробные пояснения в работе расписаны в коде.
    */
    if (node->parent->color == RB_BLACK) {
        return;
    }

    /*Берём удобные обозначения, чтобы не париться с стрелочками*/
    RBNode *ded = node->parent->parent;
    RBNode *father = node->parent;
    RBNode *uncle = NULL;
    int father_status; //0 - отец справа, 1 - отец слева
    int node_status; //0 - мы справа, 1 - мы слева
    /*Проверка ситуации: Зиг-заг или прямая линия
    */
    if (ded->left == father) {
        uncle = ded->right;
        father_status = 1;
    } else {
        uncle = ded->left;
        father_status = 0;
    }
    
    if (father->left == node) {
        node_status = 1;
    } else {
        node_status = 0;
    }

    /*Ситуация 1: Дядя красный
    В этом случае мы перекрашиваем папу и дядю в чёрный, а деда в красный,
    Теперь проблемный узел - дед, вызываем рекурсивно эту же функцию для него
    */
    if (uncle->color == RB_RED) {
        father->color = RB_BLACK;
        uncle->color = RB_BLACK;
        ded->color = RB_RED;
        balancing(ded, tree, nil);
        return;
    }

    int line = 0; //Запускает после Зиг-Зага в линию
    /*Ситуация 2: Зиг-Заг
    Мы - левый ребёнок, а наш отец - правый ребёнок (или наоборот)
    Мы меняем местами отца и нас, из-за чего статус отца и ребенка теперь одинаковый
    (Так происходит, потому левое поддерево < узел < правое поддерево)
    После этого переходим к ситуации 3
    */
    if (node_status != father_status) {
        /*Зиг-заг*/
        if (father_status == 1) {
            /*Здесь просто тупая перестановка*/
            ded->left = node;
            father->right = node->left;
            if (node->left != nil) {
                node->left->parent = father;
            }

            node->parent = ded;
            father->parent = node;
            node->left = father;
            line = 1;
            RBNode *tmp = father;
            father = node;
            node = tmp;
        } else if (father_status == 0) {
            /*Здесь тоже просто тупая перестановка отца и сына*/
            ded->right = node;
            father->left = node->right;
            if (node->right != nil) {
                node->right->parent = father;
            }

            node->parent = ded;
            father->parent = node;
            node->right = father;
            line = 1;
            RBNode *tmp = father;
            father = node;
            node = tmp;
        }
    }
    /*Ситуация 3: Прямая линия
    Нам нужно перевернуть дерево
    Красим деда и отца в красный и чёрный соответственно
    Двигаем себя на место отца, отца на место деда, деда на место дяди
    Переподкрепляем все поддеревья, чтобы было выполнено условие дерева
    (Левое поддерево < узла < правого поддерева)
    */
    if (node_status == father_status || line == 1) {
        /*Прямая линия*/
        ded->color = RB_RED;
        father->color = RB_BLACK;
        if (father_status == 1) {
            /*Папа и сын левые дети*/
            RBNode *tmp = ded->parent;
            if (ded->parent->left == ded) {
                ded->parent->left = father;
            } else if (ded->parent->right == ded) {
                ded->parent->right = father;
            } else {
                tree->root = father;
            }
            ded->parent = father;
            father->parent = tmp;
            ded->left = father->right;
            if (ded->left != nil) {
                ded->left->parent = ded;
            }

            father->right = ded;    
        } else {
            /*Папа и сын правые дети*/
            RBNode *tmp = ded->parent;
            if (ded->parent->left == ded) {
                ded->parent->left = father;
            } else if (ded->parent->right == ded) {
                ded->parent->right = father;
            } else {
                tree->root = father;
            }

            ded->parent = father;
            father->parent = tmp;
            ded->right = father->left;
            if (ded->right != nil) {
                ded->right->parent = ded;
            }
            father->left = ded;
        }
    }
    /*Всегда делаем узел чёрным, чтобы нигде ничего не портилось*/
    tree->root->color = RB_BLACK;
}

void rbInsert(RBTree* tree, const char* key, int doc_id, const char* title) {
    /*Принимает на вход указатели на дерево, ключ, заголовок и id документа
    Добавляем данные в наденный вектор либо добавляем новый узел и балансируем
    Ничего не возвращаем
    */
    if (tree == NULL || key == NULL || title == NULL) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return;
    }
    /*Ситуация 1: пустое дерево и мы добавляем корень*/
    if (tree->root == tree->nil) {
        tree->root = (RBNode *)malloc(sizeof(RBNode));
        if (tree->root == NULL) {
            printf("Err in %s:\nMemory was not allocated\n", __func__);
            return;
        }

        tree->root->color = RB_BLACK;
        tree->root->key = strdup(key);
        if (tree->root->key == NULL) {
            free(tree->root);
            tree->root = tree->nil;
            return;
        }

        tree->root->left = tree->nil;
        tree->root->right = tree->nil;
        tree->root->parent = tree->nil;
        tree->root->postings = createVector(sizeof(PostingEntry));
        if (tree->root->postings == NULL) {
            free(tree->root->key);
            free(tree->root);
            tree->root = tree->nil;
            return;
        }

        tree->size++;
        appendPosting(tree->root->postings, doc_id, title);
        return;
    }

    /*Ситуация 2: Ищем в дереве место куда добавлять*/
    RBNode *prev_node = tree->root;
    RBNode *curr_node = tree->root;
    int push = 0;
    while (curr_node != tree->nil) {
        prev_node = curr_node;
        if (strcmp(curr_node->key, key) < 0) {
            curr_node = curr_node->right;
        } else if (strcmp(curr_node->key, key) > 0) {
            curr_node = curr_node->left;
        } else {
            push = 1;
            break;
        }
    }

    if (push == 1) {
        /*Если нашли, куда добавлять*/
        appendPosting(curr_node->postings, doc_id, title);
        return;
    } else {
        /*Ситуация 3: Добавляем новый узел в дерево*/
        if (strcmp(prev_node->key, key) < 0) {
            /*Добавляем как правого ребёнка*/
            prev_node->right = (RBNode *)malloc(sizeof(RBNode));
            if (prev_node->right == NULL) {
                prev_node->right = tree->nil;
                return;
            }
            prev_node->right->color = RB_RED;
            prev_node->right->key = strdup(key);
            if (prev_node->right->key == NULL) {
                free(prev_node->right);
                prev_node->right = tree->nil;
                return;
            }

            prev_node->right->left = tree->nil;
            prev_node->right->right = tree->nil;
            prev_node->right->parent = prev_node;
            prev_node->right->postings = createVector(sizeof(PostingEntry));
            if (prev_node->right->postings == NULL) {
                free(prev_node->right->key);
                free(prev_node->right);
                prev_node->right = tree->nil;
                return;
            }

            tree->size++;
            appendPosting(prev_node->right->postings, doc_id, title);
            balancing(prev_node->right, tree, tree->nil);
            return;
        } else if (strcmp(prev_node->key, key) > 0) {
            /*Добавляем как левого ребёнка*/
            prev_node->left = (RBNode *)malloc(sizeof(RBNode));
            if (prev_node->left == NULL) {
                prev_node->left = tree->nil;
                return;
            }

            prev_node->left->color = RB_RED;
            prev_node->left->key = strdup(key);
            if (prev_node->left->key == NULL) {
                free(prev_node->left);
                prev_node->left = tree->nil;
                return;
            }

            prev_node->left->left = tree->nil;
            prev_node->left->right = tree->nil;
            prev_node->left->parent = prev_node;
            prev_node->left->postings = createVector(sizeof(PostingEntry));
            if (prev_node->left->postings == NULL) {
                free(prev_node->left->key);
                free(prev_node->left);
                prev_node->left = tree->nil;
                return;
            }

            tree->size++;
            appendPosting(prev_node->left->postings, doc_id, title);
            balancing(prev_node->left, tree, tree->nil);
            return;
        }
    }
}

Vector* rbSearch(const RBTree* tree, const char* key) {
    /*Принимаем на вход указатель на дерево и ключ
    Ищем вектор posting для этого ключа
    Возвращаем этот вектор*/
    if (tree == NULL || key == NULL) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    RBNode *curr_node = tree->root;
    while (curr_node != tree->nil) {
        /*Проверяем совпадение ключа с ключом данного узла,
        Возвращаем его postings, либо переходим к соответствующему ребёнку*/
        int comparing = strcmp(curr_node->key, key);
        if (comparing == 0) {
            return curr_node->postings;
        } else if (comparing < 0) {
            curr_node = curr_node->right;
        } else {
            curr_node = curr_node->left;
        }
    }
    /*Не нашли:(*/
    printf("Err in %s:\nData was not found\n", __func__);
    return NULL;
}

void rbTraverseRecursive(RBNode *node,
    RBNode *nil,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx) {
        if (node == nil) {
            return;
        }
        /*Принимает на вход:
            Узел, с которым работаем,
            Функцию, которую применяем к данным узла,
            Дополнительный контекст для полной универсальности
        Для красоты работаем сначала с меньшими(левыми), потом с узлом, потом с большими(правыми)
        Ничего не возвращаем*/
        rbTraverseRecursive(node->left, nil, visit, ctx);
        visit(node->key, node->postings, ctx);
        rbTraverseRecursive(node->right, nil, visit, ctx);
    }

void rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx) {
    if (tree == NULL) {
        return;
    }
    if (visit == NULL) {
        return;
    }
    /*Принимаем на вход:
        Указатель на дерево
        Функцию, которую применяем к данным узла
        Доп. контекст для универсальности
    Ничего не возвращаем
    Вызываем рекурсивную функцию для корня*/
    rbTraverseRecursive(tree->root, tree->nil, visit, ctx);
}