
/**
* Simple wrapper around malloc/free for aligned allocations to arbitrary values
*
*	Copyright (C) 2016 Andrei Danaila, All Rights Reserved
*				<mailto: adanaila >at< ctrlinux[.]com>
*	This file is part of the aligned malloc library.
	Aligned malloc is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, version 2.
*	pwm is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	If not, see <http://www.gnu.org/licenses/>.
*/
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "inttypes.h"

#define MAX_ITER 10000

void *align_malloc(size_t size, size_t align_value)
{
	size_t amnt_to_alloc;
	void *unaligned_buf;
	void *aligned_buf;

	if (align_value < sizeof(void *))
		/* Quite wasteful, lots of room for optimizing this */
		amnt_to_alloc = align_value * sizeof(void *);
	else
		amnt_to_alloc = align_value;

	amnt_to_alloc += size + sizeof(void *);

	unaligned_buf = malloc(amnt_to_alloc);
	if (!unaligned_buf)
		return NULL;

	/* Perform the initial alignment */
	aligned_buf = (void *)((uintptr_t)unaligned_buf +
			align_value - ((uintptr_t)unaligned_buf % align_value));

	/* If we cannot fit our pointer, move one more alignment value */
	while ((aligned_buf - unaligned_buf) < sizeof(void *))
		aligned_buf = (void *)((uintptr_t)aligned_buf + align_value);

	/* Populate the unaligned address before the start of the aligned buffer
	 * to facilitate freeing later */
	*(uintptr_t *)((uintptr_t)aligned_buf - sizeof(void *)) =
						     (uintptr_t)unaligned_buf;
	return aligned_buf;
}

void align_free(void *aligned_ptr)
{
	free(*(void **)((uintptr_t)aligned_ptr - sizeof(void *)));
}

int main ()
{
	size_t size, align_value;
	char *align_buffer = NULL;
	int i = 0;

	/* Test the allocator */
	for (i == 0; i < MAX_ITER; ++i) {
		size = rand() % 512 + 1;
		align_value = rand() % 512 + 1;
		align_buffer = align_malloc(size, align_value);
		if (!align_buffer)
			return -ENOMEM;
		assert(!((size_t)align_buffer % align_value));
		align_free(align_buffer);
	}
	return 0;
}

