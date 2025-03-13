#ifndef ENCODE_H
#define ENCODE_H

#include "simpleArray.h"

#include <stdbool.h>
#include <stdint.h>

int32_t get_samples(char filename[], int32_t argc, _Bool is_first, int32_t *sample_rate, Array *res);

#endif	  // ENCODE_H
