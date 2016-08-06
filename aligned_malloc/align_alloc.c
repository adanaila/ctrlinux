/**
 * Simple wrapper around malloc/free for aligned allocations to arbitrary
 * alignment values
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
#include "inttypes.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

/* Leverage the stdlib malloc to allocate aligned buffers.
 * 	input: desired buffer size, desired alignment value
 * 	Idea:
 * 		Pad the user desired size with the alignment value and enough
 * 		space to insert a void* before the aligned address.
 * 	Corner Cases:
 * 		1. malloc() returned address is already aligned to user spec
 * 		   and we cannot fit our unaligned pointer before it.
 * 		Move aligned address to the next alignment value insert pointer
 * 		before that.
 * 		2. malloc() returns an unaligned buffer but we have no space
 * 		to add the unalligned pointer before the aligned address,
 * 		Move the aligned address tothe next alignment value
 * 		insert pointer before that
 */
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
	while (((uintptr_t)aligned_buf - (uintptr_t)unaligned_buf) <
								sizeof(void *))
		aligned_buf = (void *)((uintptr_t)aligned_buf + align_value);

	/* Populate the unaligned address before the start of the aligned buffer
	 * to facilitate freeing later */
	*(uintptr_t *)((uintptr_t)aligned_buf - sizeof(void *)) =
						     (uintptr_t)unaligned_buf;
	/* Bounds checking  on the aligned buffer */
	assert((uintptr_t)aligned_buf + size <=
				(uintptr_t)unaligned_buf + amnt_to_alloc);
	/* Check the cookie bounds */
	assert(((uintptr_t)aligned_buf - sizeof(void *)) >=
				(uintptr_t)unaligned_buf);

	return aligned_buf;
}

/* The unaligned pointer is located just before the aligned_ptr
 * We can call free on it and let free clean up the memory after us
 * 	input: aligned pointer previously obtained from align_malloc()
 */
void align_free(void *aligned_ptr)
{
	free(*(void **)((uintptr_t)aligned_ptr - sizeof(uintptr_t)));
}

