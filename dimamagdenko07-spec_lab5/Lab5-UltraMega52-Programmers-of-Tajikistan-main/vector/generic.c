#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "generic.h"

bool needToResize(Vector *vector, bool *increase) {
    if (vector == NULL || increase == NULL) return false;

    if (vector->size == vector->capacity) {
        *increase = true; // Надо увеличить размер
        return true;
    }
    if (vector->size > 0 && vector->size <= vector->capacity / 4 && vector->capacity > MIN_SIZE) {
        *increase = false; // Надо уменьшить размер
        return true;
    }
    return false;
}

int resize(Vector *vector, bool increase) {
    if (vector == NULL) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return -1;
    }

    size_t new_capacity;
    if (increase) {
        new_capacity = (vector->capacity > 0) ? vector->capacity * 2 : MIN_SIZE;
    } else {
        new_capacity = vector->capacity / 2;
        if (new_capacity < MIN_SIZE) {
            new_capacity = MIN_SIZE;
        }
    }

    void *new_data = realloc(vector->data, new_capacity * vector->elem_size);  // Реаллоцируем вектор в новое место
    if (new_data == NULL) {
        printf("Err in %s:\nSomething went\n", __func__);
        return -2;
    }

    vector->data = new_data;
    vector->capacity = new_capacity;
    return 0;
}

Vector *createVector(size_t elem_size) {
    if (elem_size == 0) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return NULL;
    }

    Vector *vector = malloc(sizeof(Vector));
    if (!vector) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    vector->elem_size = elem_size;
    vector->size = 0;
    vector->capacity = MIN_SIZE;

    vector->data = malloc(vector->capacity * vector->elem_size);
    if (!vector->data) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        free(vector);
        return NULL;
    }

    return vector;
}

int appendVectorItem(Vector *vector, void *el) {
    if (vector == NULL || el == NULL) {
        printf("Err in %s:\nSome data is empty\n", __func__);
        return -1;
    }

    bool increase;
    if (needToResize(vector, &increase)) {
        if (resize(vector, increase) != 0) {
            printf("Err in %s:\nDidn't manage to resize\n", __func__);
            return -2;
        }
    }

    void *dest = (char *)vector->data + vector->size * vector->elem_size;
    memcpy(dest, el, vector->elem_size);
    vector->size++;

    return 0;
}

void *getVectorItem(Vector *vector, size_t index) {
    if (index >= vector->size) {
        printf("Err in %s:\nInvalid data\n", __func__);
        return NULL;
    }

    return (char *)vector->data + index * vector->elem_size;
}

int setVectorItem(Vector *vector, size_t index, void *value) {
    if (index >= vector->size) {
        printf("Err in %s:\nInvalid data\n", __func__);
        return -1;
    }

    void *dest = (char *)vector->data + index * vector->elem_size;
    memcpy(dest, value, vector->elem_size);
    return 0;
}

void *popVectorItem(Vector *vector, size_t index) {
    if (vector->size == 0 || index >= vector->size) {
        printf("Err in %s:\nInvalid data\n", __func__);
        return NULL;
    }

    void *copy = malloc(vector->elem_size);
    if (!copy) {
        printf("Err in %s:\nMemory was not allocated\n", __func__);
        return NULL;
    }

    void *item_to_pop = (char *)vector->data + index * vector->elem_size;
    memcpy(copy, item_to_pop, vector->elem_size);

    if (index < vector->size - 1) {
        void *from = (char *)vector->data + (index + 1) * vector->elem_size;
        size_t bytes_to_move = (vector->size - (index + 1)) * vector->elem_size;
        memmove(item_to_pop, from, bytes_to_move);
    }

    vector->size -= 1;

    bool increase;
    if (needToResize(vector, &increase)) {
        if (resize(vector, increase) != 0) {
            printf("Err in %s:\nDidn't manage to resize\n", __func__);
            return NULL;
        }
    }

    return copy;
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp) {
    if (vector->size == 0) {
        printf("Err in %s:\nInvalid data\n", __func__);
        return -2;
    }

    for (size_t i = 0; i < vector->size; i++) {
        void *elem = (char *)vector->data + i * vector->elem_size;
        if (cmp(elem, value)) return (long int)i;
    }
    // Элемент не найден
    printf("Err in %s:\nElement was not found\n", __func__);
    return -1;
}

void vectorFree(Vector *vector) {
    if (!vector) return;

    free(vector->data);
    free(vector);

    return;
}
