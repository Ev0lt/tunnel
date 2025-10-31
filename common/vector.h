#ifndef VECTOR_H
#define VECTOR_H 

typedef struct Vector {
    void *data;//数据指针
    int VectorSize;//这个Vector的初始大小
    int Volume;//这个Vector中放了多少数据
    int dataSize;//每个数据的大小
} Vector;

Vector *initVector(int size);

void vAdd(Vector *v,void *data);

void* vGet(Vector *v,int index);

int getIndex(Vector *v,void *data);

/* Insert data at index (0..Volume). Returns 0 on success, -1 on error. */
int vInsert(Vector *v,int index,void *data);

void* vDel(Vector *v,int index);

#endif