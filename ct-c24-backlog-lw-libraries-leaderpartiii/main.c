#include "include/correlation.h"
#include "include/encode.h"
#include "return_codes.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Using : %s \n", argv[0]);
		return ERROR_ARGUMENTS_INVALID;
	}
	int32_t delta, delta_t, sample_rate_1, sample_rate_2, code_error = SUCCESS;
	Array res;
	init_array(&res);

	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "Incorrect number of arguments expected 2 or 3 but actual %d ", argc);
		return ERROR_ARGUMENTS_INVALID;
	}

	if ((code_error = get_samples(argv[1], argc, false, &sample_rate_1, &res)))
	{
		return code_error;
	}

	res.size1 = res.size;
	if ((code_error = get_samples(argc == 2 ? argv[1] : argv[2], argc, argc == 2 ? true : false, &sample_rate_2, &res)))
	{
		return code_error;
	}

	if (sample_rate_1 != sample_rate_2)
	{
		free_array_with_size(&res);
		fprintf(stderr, "Sorry,  but your sample rate is different 1 = %d ,\t 2=%d ", sample_rate_1, sample_rate_2);
		return ERROR_DATA_INVALID;
	}
	code_error = correlation(&res, &delta);
	if (code_error)
	{
		return code_error;
	}
	delta_t = (int32_t)((double)delta / sample_rate_1 * 1000);
	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", delta, sample_rate_1, delta_t);
	return SUCCESS;
}
