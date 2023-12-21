#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define VECTOR_INIT_SIZE 16
#define VECTOR_SIZE_INCREASE 1.5

typedef struct VECTOR {
    void* v;
    size_t bytewidth;
    int size;
    int head;
    int tail;
} VECTOR;

VECTOR construct_vector(size_t bytewidth);
void deconstruct_vector(VECTOR vector);
bool vector_is_empty(VECTOR* vector);
int vector_length(VECTOR* vector);
void vector_increase_size(VECTOR* vector);
void vector_enqueue(VECTOR* vector, void* data);
int vector_dequeue(VECTOR* vector, void* data);
int vector_index(VECTOR* vector, void* data, bool (*equal)(void*, void*));
int vector_local_index_to_global_index(VECTOR* vector, int index);
void vector_get(VECTOR* vector, int index, void* data);
void vector_iterate(VECTOR* vector, bool (*func)(void*, void*), void* params);
bool vector_filter_func(void* data, void* params);
VECTOR vector_filter(VECTOR* vector, bool (*filter_func)(void*, void*), void* params);

#endif
