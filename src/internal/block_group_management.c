/*
page_management.c

Copyright (c) 09 Yann BOUCHER (yann)

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

#include "block_group_management.h"

#include <stddef.h> /* NULL */

#include "os_interface.h"

#include "debug.h"

block_group* first_block_group;
int yam_allocated_memory;
/* a factor of 7 seems to reasonably reduce the amount of fragmentation */
int yam_page_allocation_factor = 7;

extern int page_size;

block_group *alloc_block_group(size_t byte_len)
{
    byte_len += sizeof(block_group); /* take in account the size of the record*/

    size_t page_count = byte_len/page_size + (byte_len%page_size?1:0);
    page_count*=yam_page_allocation_factor;

    void* new_page = (void*)os_alloc_pages(page_count);
    if (!new_page) return NULL;

    /* allocated pages are automatically cleared to 0, no need to use memset */
    block_group* record = new_page;

    const size_t block_size = page_count*page_size - sizeof(block_group);

    record->page_count = page_count;
    record->largest_free_block = block_size;
    record->checksum = block_group_checksum(record);

    record->first_block->block_group = record;
    record->first_block->size = block_size;
    record->first_block->checksum = block_checksum(record->first_block);

    yam_allocated_memory += page_count*page_size;

    return record;
}

void free_block_group(block_group *record)
{
    ASSERT(record);

    yam_allocated_memory -= page_size*record->page_count;

    os_free_pages((uintptr_t)record, record->page_count);
}

block *find_largest_free_block(block_group *bg)
{
    block* current_block = bg->first_block;
    block* largest_block = current_block;
    while (current_block)
    {
        if (!current_block->used && (largest_block->used || current_block->size > largest_block->size))
            largest_block = current_block;

        ASSERT(current_block->size);
        current_block = next_block(current_block);
    }

    if (largest_block->used) /* we didn't find any free block */
    {
        return NULL;
    }
    return largest_block;
}

void delete_block_group(block_group *record)
{
    if (record->prev_block_group && record->next_block_group)
    {
        record->prev_block_group->next_block_group = record->next_block_group;
        record->prev_block_group->checksum
                = block_group_checksum(record->prev_block_group);
        record->next_block_group->prev_block_group = record->prev_block_group;
        record->next_block_group->checksum
                = block_group_checksum(record->next_block_group);
    }
    else if (!record->prev_block_group)
    {
        first_block_group = record->next_block_group;
        if (record->next_block_group)
        {
            record->next_block_group->prev_block_group = NULL;
            record->next_block_group->checksum
                    = block_group_checksum(record->next_block_group);
        }
    }
    else // !record->next_block_group
    {
        record->prev_block_group->next_block_group = NULL;
        record->prev_block_group->checksum
                = block_group_checksum(record->prev_block_group);
    }

    free_block_group(record);
}

size_t used_block_count(const block_group* bg)
{
    const block* current_block = bg->first_block;
    size_t counter = 0;
    while (current_block)
    {
        if (!check_block(current_block))
        {
            dump_yam_state();
            fflush(stdout);
        }
        ASSERT(check_block(current_block));
        if (current_block->used) ++counter;

        current_block = next_block(current_block);
    }

    return counter;
}
