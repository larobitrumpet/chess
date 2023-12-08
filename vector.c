#include "vector.h"

static inline int mod(int a, int b) {
    return a < 0 ? a % b + b : a % b;
}

VECTOR construct_vector(size_t bytewidth) {
    VECTOR vector;
    vector.bytewidth = bytewidth;
    vector.size = VECTOR_INIT_SIZE;
    vector.v = malloc(vector.bytewidth * vector.size);
    vector.head = 0;
    vector.tail = 0;
    return vector;
}

void deconstruct_vector(VECTOR vector) {
    free(vector.v);
}

bool vector_is_empty(VECTOR* vector) {
    return vector->head == vector->tail;
}

int vector_length(VECTOR* vector) {
    return mod(vector->tail - vector->head, vector->size);
}

void vector_increase_size(VECTOR* vector) {
    int old_size = vector->size;
    vector->size = (int)(old_size * VECTOR_SIZE_INCREASE);
    void* new_q = malloc(vector->bytewidth * vector->size);
    int i = vector->head;
    int j = 0;
    while (i != vector->tail) {
        memcpy(new_q + j * vector->bytewidth, vector->v + i * vector->bytewidth, vector->bytewidth);
        i = mod(i + 1, old_size);
        j++;
    }
    free(vector->v);
    vector->v = new_q;
    vector->head = 0;
    vector->tail = j;
}

void vector_enqueue(VECTOR* vector, void* data) {
    if (vector->head == mod(vector->tail + 1, vector->size))
        vector_increase_size(vector);
    memcpy(vector->v + vector->tail * vector->bytewidth, data, vector->bytewidth);
    vector->tail = mod(vector->tail + 1, vector->size);
}

int vector_dequeue(VECTOR* vector, void* data) {
    if (vector_is_empty(vector)) {
        return 1;
    }
    memcpy(data, vector->v + vector->head * vector->bytewidth, vector->bytewidth);
    vector->head = mod(vector->head + 1, vector->size);
    return 0;
}

int vector_index(VECTOR* vector, void* data, bool (*equal)(void*, void*)) {
    int index = 0;
    int i = vector->head;
    while (i != vector->tail) {
        if (equal(vector->v + i * vector->bytewidth, data)) {
            return index;
        }
        i = mod(i + 1, vector->size);
        index++;
    }
    return -1;
}

int vector_local_index_to_global_index(VECTOR* vector, int index) {
    return mod(vector->head + index, vector->size);
}

void vector_get(VECTOR* vector, int index, void* data) {
    memcpy(data, vector->v + vector_local_index_to_global_index(vector, index) * vector->bytewidth, vector->bytewidth);
}

//void vector_remove(VECTOR* vector, int index) {
//    if (index >= vector_length(vector) || index < 0)
//        return;
//    int i = vector_local_index_to_global_index(index);
//    while (i != vector->tail) {
//        memcpy(vector->v + i * vector->bytewidth, vector->v + mod(i + 1, vector->size) * vector->bytewidth, vector->bytewidth);
//        i = mod(i + 1, vector->size);
//    }
//    vector->tail = mod(vector->tail - 1, vector->size);
//}

void vector_iterate(VECTOR* vector, bool (*func)(void*, void*), void* params) {
    int i = vector->head;
    while (i != vector->tail) {
        if (func(vector->v + i * vector->bytewidth, params))
            break;
        i = mod(i + 1, vector->size);
    }
}

//bool vector_filter_func(void* data, void* params) {
//    void** pars = (void**)params;
//    bool (*filter_func)(void*, void*) = (bool (*)(void*, void*))pars[0];
//    VECTOR* vector = (VECTOR*)pars[1];
//    void* par = pars[2];
//    if (filter_func(data, par)) {
//        vector_enqueue(vector, data);
//    }
//    return false;
//}
//
//VECTOR vector_filter(VECTOR* vector, bool (*filter_func)(void*, void*), void* params) {
//    VECTOR new_vector = construct_vector(vector->bytewidth);
//    void* pars[3] = {(void*)filter_func, (void*)&new_vector, params};
//    vector_iterate(vector, vector_filter_func, (void*)pars);
//}
