#include "vector.h"
#include <string.h>
#include <stdlib.h>
Vector *initVector(int size){
    Vector *v = (Vector *)malloc(sizeof(Vector));
    v->VectorSize=10; /* initial capacity (number of elements) */
    v->Volume=0;
    v->dataSize = size;
    v->data = malloc(size * v->VectorSize);
    return v;
}

void vAdd(Vector *v,void *data){
    /* Expand if full */
    if (v->Volume >= v->VectorSize) {
        int newSize = v->VectorSize + 10;
        void *ndata = malloc(newSize * v->dataSize);
        if (!ndata) {
            /* allocation failed - leave unchanged */
            return;
        }
        memcpy(ndata, v->data, (size_t)v->dataSize * v->Volume);
        free(v->data);
        v->data = ndata;
        v->VectorSize = newSize;
    }
    /* copy new element into next slot */
    memcpy((char *)v->data + (size_t)v->Volume * v->dataSize, data, v->dataSize);
    v->Volume++;
}

void* vGet(Vector *v,int index){
    if (index < 0 || index >= v->Volume){
        return NULL;
    }
    return (char *)v->data + (size_t)index * v->dataSize;
}

int getIndex(Vector *v,void *data){
    /* Compare contents rather than pointer equality */
    for (int i = 0; i < v->Volume; ++i) {
        void *elem = vGet(v, i);
        if (elem && memcmp(elem, data, v->dataSize) == 0) {
            return i;
        }
    }
    return -1;
}

int vInsert(Vector *v,int index,void *data){ 
    if (!v) return -1;
    if (index < 0 || index > v->Volume) return -1; /* allow insert at end (index==Volume) */

    /* expand if necessary */
    if (v->Volume >= v->VectorSize) {
        int newSize = v->VectorSize + 10;
        void *ndata = malloc((size_t)newSize * v->dataSize);
        if (!ndata) return -1;
        memcpy(ndata, v->data, (size_t)v->dataSize * v->Volume);
        free(v->data);
        v->data = ndata;
        v->VectorSize = newSize;
    }

    /* shift elements to make space for the new element */
    if (index < v->Volume) {
        memmove((char *)v->data + (size_t)(index + 1) * v->dataSize,
                (char *)v->data + (size_t)index * v->dataSize,
                (size_t)v->dataSize * (v->Volume - index));
    }

    /* copy new element into slot */
    memcpy((char *)v->data + (size_t)index * v->dataSize, data, v->dataSize);
    v->Volume++;
    return 0;
}

void vSet(Vector *v,int index,void *data){
    void *elem = vGet(v, index);
    if (elem) memcpy(elem, data, v->dataSize);
}

void* vDel(Vector *v,int index){
    void *src = vGet(v, index);
    if (src == NULL) {
        return NULL;
    }
    /* Make a copy of the removed element to return */
    void *removed = malloc(v->dataSize);
    if (removed) memcpy(removed, src, v->dataSize);

    /* Shift remaining elements down */
    if (index < v->Volume - 1) {
        memmove((char *)v->data + (size_t)index * v->dataSize,
                (char *)v->data + (size_t)(index + 1) * v->dataSize,
                (size_t)v->dataSize * (v->Volume - index - 1));
    }
    v->Volume--;
    return removed;
}