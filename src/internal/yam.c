/*
yam.c

Copyright (c) 13 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "../yam.h"

#include "block_alloc.h"
#include "block_group_record.h"
#include "init.h"
#include "os_interface.h"
#include "debug.h"

#define ALIGNMENT 16
#define CONTROL_MAGIC 0xBEEF

typedef struct __attribute__((packed)) control_structure
{
    block* associated_block;
    uint16_t magic;
} control_structure;

int yam_used_memory = 0;

void* yam_memcpy(void* restrict dstptr, const void* restrict srcptr, size_t n)
{
    uint8_t* dst = dstptr;
    const uint8_t* src = srcptr;
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
    return dstptr;
}

block* get_block(void* ptr)
{
    control_structure* control =
            (control_structure*)((uint8_t*)ptr - sizeof(control_structure));

    if (control->magic != CONTROL_MAGIC)
    {
        ERROR("Invalid control magic : address %p is invalid\n", ptr);
        return NULL;
    }

    ASSERT(control->associated_block->magic == BLOCK_MAGIC);

    return control->associated_block;
}

size_t content_size(void* ptr)
{
    block* blk = get_block(ptr);
    const size_t content_size = ((uintptr_t)blk + blk->size) - (uintptr_t)ptr;

    return content_size;
}

void* first_aligned_boundary(void* base, size_t alignment)
{
    while ((uintptr_t)base % alignment)
    {
        base = (uint8_t*)base + 1;
    }

    return base;
}

void *yam_aligned_alloc(uint32_t size, size_t alignement)
{
    os_lock();

    if (!yam_initialized)
    {
        init();
    }

    /* Add the alignment requirement so it's always possible to return aligned
     * data, and have enough space to fit the control structure               */
    const uint32_t alloc_size = size+alignement+
            sizeof(control_structure)+sizeof(block);

    block* block = allocate_new_block(alloc_size);
    if (!block)
    {
        os_unlock();
        return NULL;
    }

    void* address = first_aligned_boundary(
                (uint8_t*)block->data + sizeof(control_structure), alignement);

    control_structure* control = (control_structure*)
            ((uint8_t*)address - sizeof(control_structure));

    control->associated_block = block;
    control->magic = CONTROL_MAGIC;

    ASSERT(control->associated_block->magic == BLOCK_MAGIC);
    ASSERT(control->associated_block->block_group->magic == BLOCK_GROUP_MAGIC);

    /* check if the alignment is correct */
    ASSERT((uintptr_t)address % alignement == 0);

    yam_used_memory += block->size;

    os_unlock();

    return address;
}

void *yam_alloc(uint32_t size)
{
    return yam_aligned_alloc(size, ALIGNMENT);
}

void yam_free(void *address)
{
    if (address == NULL) return;

    block* blk = get_block(address);
    if (!blk) return;

    os_lock();

    yam_used_memory -= blk->size;

    ASSERT(blk->used);
    delete_block(blk);

    os_unlock();
}

void *yam_realloc(void *ptr, size_t size)
{
    if (size == 0)
    {
        yam_free(ptr);
        return NULL;
    }

    block* blk = get_block(ptr);
    if (!blk) return NULL;


    os_lock();
    int result = resize_block(blk, size);
    os_unlock();
    if (!result)
    {
        void* copy = yam_alloc(size);
        yam_memcpy(copy, ptr, content_size(ptr));
        yam_free(ptr);

        return copy;
    }
    else
    {

        return ptr;
    }
}
