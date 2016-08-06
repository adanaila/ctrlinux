#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "align_alloc.h"

#define MAX_ITER 100000000

int main()
{
	size_t size, align_value;
	char *align_buffer = NULL;
	int i = 0;

	/* Test the allocator */
	for (i = 0; i < MAX_ITER; ++i) {
		size = rand() % 4096 + 1;
		align_value = rand() % 4096 + 1;
		align_buffer = align_malloc(size, align_value);
		if (!align_buffer)
			return -ENOMEM;
		assert(!((size_t)align_buffer % align_value));
		align_free(align_buffer);
	}
	return 0;
}
