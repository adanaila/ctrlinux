#ifndef _ALIGN_ALLOC_H_
#define _ALIGN_ALLOC_H_
extern void *align_malloc(size_t size, size_t align_value);
extern void align_free(void *aligned_ptr);
#endif
