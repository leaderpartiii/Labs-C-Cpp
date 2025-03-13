#include "../include/simpleArray.h"

#include "../return_codes.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIALIZE_SIZE 64

void init_array(Array *arr)
{
	arr->size = 0;
	arr->capacity = 0;
	arr->array = NULL;
}

void free_array_with_size(Array *arr)
{
	if (arr->array)
		free(arr->array);
	arr->size = 0;
	arr->capacity = 0;
}

int32_t resize_array(Array *array, size_t new_size)
{
	double *new_array = realloc(array->array, new_size * sizeof(double));
	if (new_array == NULL)
	{
		fprintf(stderr, "Could not allocate memory for the array \n");
		free_array_with_size(array);
		return ERROR_NOTENOUGH_MEMORY;
	}
	array->capacity = new_size;
	array->array = new_array;
	return SUCCESS;
}

int32_t add(Array *array, double element)
{
	int32_t code_error;

	if (array->capacity <= array->size &&
		(code_error = resize_array(array, array->capacity == 0 ? (size_t)INITIALIZE_SIZE : array->capacity * 2)) != SUCCESS)
	{
		fprintf(stderr, "Could not allocate size %zu\n", array->capacity == 0 ? (size_t)INITIALIZE_SIZE : array->capacity * 2);
		return code_error;
	}
	array->array[array->size++] = element;
	return SUCCESS;
}
