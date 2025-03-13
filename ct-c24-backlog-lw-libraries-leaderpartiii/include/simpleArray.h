#ifndef SIMPLEARRAY_H
#define SIMPLEARRAY_H

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
	double *array;
	size_t size;	 // size for all elements
	size_t size1;	 // size for certain number elements
	size_t capacity;
} Array;

void init_array(Array *arr);

void free_array_with_size(Array *arr);

int32_t resize_array(Array *array, size_t new_size);

int32_t add(Array *array, double element);

#endif	  // SIMPLEARRAY_H
