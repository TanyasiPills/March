#ifndef MAYOS

#pragma once

#include "bare.h"


typedef struct Vector {
    unsigned char* data;
    uint64_t size;
    uint64_t capacity;
    uint64_t typeSize;
} Vector;

/** Initialize the vector with given capacity and element type size.
 * @param v Pointer to the vector structure.
 * @param size Initial maximum number of elements the vector can hold.
 * @param typeSize Size in bytes of each element.
 */
void VectorInit(Vector* v, uint64_t size, uint64_t typeSize);

/** Clear all elements from the vector and free memory.
 * @param v Pointer to the vector structure.
 */
void VectorClear(Vector* v);

/** Add an element to the end of the vector. If full, doubles capacity.
 * @param v Pointer to the vector structure.
 * @param item Pointer to the element to add (must be typeSize bytes).
 */
void VectorPush(Vector* v, void* item);

/** Remove and return the last element from the vector.
 * @param v Pointer to the vector structure.
 * @param item Pointer to store the removed element (must be typeSize bytes).
 */
void VectorPop(Vector* v, void* item);

/** Get an element at the specified index.
 * @param v Pointer to the vector structure.
 * @param index Index of the element to retrieve.
 * @param item Pointer to store the element (must be typeSize bytes).
 */
void VectorGet(Vector* v, uint64_t index, void* item);

/** Set an element at the specified index.
 * @param v Pointer to the vector structure.
 * @param index Index of the element to set.
 * @param item Pointer to the new element value (must be typeSize bytes).
 */
void VectorSet(Vector* v, uint64_t index, void* item);

/** Resize the vector to a new capacity.
 * @param v Pointer to the vector structure.
 * @param newCapacity New maximum number of elements the vector can hold.
 */
void VectorResize(Vector* v, uint64_t newCapacity);

/** Shrink vector capacity if size is less than half of capacity and capacity exceeds threshold.
 * @param v Pointer to the vector structure.
 * @param shrinkThreshold Minimum capacity threshold before shrinking.
 */
void VectorShrinkToFit(Vector* v, uint64_t shrinkThreshold);

/** Get the current capacity of the vector.
 * @param v Pointer to the vector structure.
 * @return Current maximum number of elements the vector can hold.
 */
uint64_t VectorGetCapacity(const Vector* v);

/** Check if the vector is empty.
 * @param v Pointer to the vector structure.
 * @return true if the vector has no elements, false otherwise.
 */
bool VectorIsEmpty(const Vector* v);

/** Get the current number of elements in the vector.
 * @param v Pointer to the vector structure.
 * @return Current number of elements in the vector.
 */
uint64_t VectorGetSize(const Vector* v);

/** Free all memory allocated for the vector.
 * @param v Pointer to the vector structure.
 */
void VectorFree(Vector* v);

#endif
