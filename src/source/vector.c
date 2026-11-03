#include "vector.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>

void VectorInit(Vector* v, uint64_t capacity, uint64_t typeSize) {
    if (v == NULL || typeSize == 0) {
        Log("Invalid parameters for VectorInit");
    }
    v->data = malloc(capacity * typeSize);
    v->size = 0;
    v->capacity = capacity;
    v->typeSize = typeSize;
}

void VectorPush(Vector* v, void* item) {
    if (v->size >= v->capacity) {
        VectorResize(v, v->capacity * 2);
    }
    memcpy(v->data + v->size * v->typeSize, item, v->typeSize);
    v->size++;
}

void VectorPop(Vector* v, void* item) {
    if (v->size == 0) {
        Log("Vector is empty, cannot pop");
        return;
    }
    v->size--;
    if (item != NULL) memcpy(item, v->data + v->size * v->typeSize, v->typeSize);
}

void VectorGet(Vector* v, uint64_t index, void* item) {
    if (index >= v->size) {
        Log("Vector index out of bounds");
        return;
    }
    memcpy(item, v->data + index * v->typeSize, v->typeSize);
}

void VectorSet(Vector* v, uint64_t index, void* item) {
    if (index >= v->size) {
        Log("Vector index out of bounds");
        return;
    }
    memcpy(v->data + index * v->typeSize, item, v->typeSize);
}

void VectorResize(Vector* v, uint64_t newCapacity) {
    void* newData = malloc(newCapacity * v->typeSize);
    uint64_t elementsToCopy = (v->size < newCapacity) ? v->size : newCapacity;
    memcpy(newData, v->data, elementsToCopy * v->typeSize);
    free(v->data);
    v->data = newData;
    v->capacity = newCapacity;
}

uint64_t VectorGetCapacity(const Vector* v) {
    return v->capacity;
}

bool VectorIsEmpty(const Vector* v) {
    return v->size == 0;
}

uint64_t VectorGetSize(const Vector* v) {
    return v->size;
}

void VectorFree(Vector* v) {
    if (v == NULL || v->data == NULL) {
        Log("Invalid parameters for VectorFree");
    }
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}
