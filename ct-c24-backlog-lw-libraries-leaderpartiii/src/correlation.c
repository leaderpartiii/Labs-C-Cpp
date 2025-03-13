#include "../include/simpleArray.h"
#include "../return_codes.h"

#include <fftw3.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int32_t fft(fftw_complex *in, fftw_complex *out, int32_t sign, size_t size)
{
	fftw_plan p = fftw_plan_dft_1d((int32_t)size, in, out, sign, FFTW_ESTIMATE);
	if (p == NULL)
	{
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(p);
	fftw_destroy_plan(p);
	fftw_cleanup();
	return SUCCESS;
}

int32_t correlation(Array *a, int32_t *result)
{
	int32_t code_error = SUCCESS;
	fftw_complex *data = NULL;
	fftw_complex *fa = NULL;
	fftw_complex *fb = NULL;
	fftw_complex *fc = NULL;
	fftw_complex *cc = NULL;
	size_t max_index = 0;

	size_t n = a->size1;
	size_t m = a->size - a->size1;
	size_t new_size = n + m;

	if ((data = fftw_alloc_complex(4 * new_size)) == NULL)
	{
		code_error = ERROR_NOTENOUGH_MEMORY;
		goto free;
	}
	fa = data;
	fb = data + new_size;
	fc = data + 2 * new_size;
	cc = data + 3 * new_size;

	for (size_t i = 0; i < n; ++i)
	{
		fa[i][0] = a->array[i];
		fa[i][1] = 0;
	}
	memset(fa + n, 0, (m) * sizeof(a->array[0]));

	for (size_t i = 0; i < m; ++i)
	{
		fb[i][0] = a->array[n + i];
		fb[i][1] = 0;
	}
	memset(fb + m, 0, (n) * sizeof(a->array[0]));

	if ((code_error = fft(fa, fa, FFTW_FORWARD, new_size)))
	{
		goto free;
	}

	if ((code_error = fft(fb, fb, FFTW_FORWARD, new_size)))
	{
		goto free;
	}

	for (size_t i = 0; i < new_size; ++i)
	{
		//(a-ib)*(c+id) = (ac+bd) + i(-cb + ad)
		fc[i][0] = (fa[i][0] * fb[i][0]) + (fa[i][1] * fb[i][1]);
		fc[i][1] = -(fa[i][0] * fb[i][1]) + (fa[i][1] * fb[i][0]);
	}

	if ((code_error = fft(fc, cc, FFTW_BACKWARD, new_size)))
	{
		goto free;
	}

	double max_val = -1.0;
	for (size_t i = 0; i < new_size; ++i)
	{
		double current_val = cc[i][0];
		if (current_val > max_val)
		{
			max_val = current_val;
			max_index = i;
		}
	}
	if (max_index >= new_size / 2)
	{
		max_index = max_index - new_size;
	}

free:
	free_array_with_size(a);
	if (data)
		fftw_free(data);

	*result = (int32_t)max_index;
	return code_error;
}
