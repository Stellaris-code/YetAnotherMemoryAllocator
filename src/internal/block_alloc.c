/*
alloc.c

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

#include "block_alloc.h"

#include "block_record.h"
#include "block_group_record.h"
#include "block_group_management.h"
#include "utils.h"

#include "debug.h"

extern block_group* first_block_group;
extern int page_size;

void mark_as_used(block *target_block, size_t size)
{
    const size_t free_size = target_block->size - size;

    block* used_block = target_block;
    if (used_block->magic != BLOCK_MAGIC)
    {
        ERROR("Invalid block magic : %p\n", used_block);
        ASSERT(0);
    }

    if (free_size <= sizeof(block)) /* not enough space for a free block */
    {
        size += free_size;
    }

    /* Mark first part of the block as used */
    used_block->size = size;
    used_block->used = 1;

    block* free_block = next_block(used_block);

    if (free_block && free_size > sizeof(block))
    {
        /* Mark the second part of the block as free */
        free_block->magic = BLOCK_MAGIC;
        free_block->size = free_size;
        free_block->used = 0;
        free_block->block_group = used_block->block_group;
        free_block->previous = used_block;
        if (next_block(free_block))
        {
            next_block(free_block)->previous = free_block;
        }
    }
}

int resize_block(block *blk, uint32_t size)
{
    ASSERT(size > sizeof(block));

    if (size > blk->size)
    {
        /* can't expand, no free block ahead */
        if (!next_block(blk) || next_block(blk)->used)
        {
            return 0;
        }
        /* not enough space */
        if ((uint32_t)blk->size + next_block(blk)->size < size)
        {
            return 0;
        }
    }
    /* we can always shrink the block */

    mark_as_used(blk, size);

    return 1;
}

block *allocate_new_block(uint32_t size)
{
    ASSERT(size > sizeof(block));

    if (first_block_group == NULL)
        first_block_group = alloc_block_group(size);

    block_group *current_bg = first_block_group;
    if (!current_bg) return NULL; /* out of memory */

    while (current_bg->largest_free_block < size)
    {
        if (!current_bg->next_block_group) break;
        current_bg = current_bg->next_block_group;
    }

    if (current_bg->magic != BLOCK_GROUP_MAGIC)
    {
        ERROR("Invalid Block group magic : %p\n", current_bg);
        ASSERT(0);
    }

    /* need to allocate a new block group */
    if (current_bg->largest_free_block < size)
    {
        current_bg->next_block_group = alloc_block_group(size);
        /* out of memory */
        if (!current_bg->next_block_group) return NULL;

        current_bg->next_block_group->prev_block_group = current_bg;
        current_bg = current_bg->next_block_group;
    }

    block* previous_block = NULL;
    block* current_block = current_bg->first_block;
    while (current_block)
    {
        if (!current_block->used && current_block->size >= size)
        {
            mark_as_used(current_block, size);
            break;
        }

        previous_block = current_block;
        current_block = next_block(current_block);
    }

    /* not found */
    if ((uintptr_t)current_block == block_group_end(current_bg))
    {
        return NULL;
    }

    current_block->magic = BLOCK_MAGIC;
    current_block->previous = previous_block;
    current_block->block_group = current_bg;

    block* largest_free_block = find_largest_free_block(current_bg);

    if (largest_free_block)
        current_bg->largest_free_block = largest_free_block->size;
    else
        current_bg->largest_free_block = 0;

    if (current_block->previous) ASSERT(next_block(current_block->previous) == current_block);
    if (next_block(current_block)) ASSERT(next_block(current_block)->previous == current_block);

    return current_block;
}

void merge_left(block* target_block);
void merge_right(block* target_block);
void merge_middle(block* target_block);

void delete_block(block *blk)
{
    if (blk->magic != BLOCK_MAGIC)
    {
        ERROR("Invalid block magic : %p\n", blk);
        ASSERT(0);
    }
    if (!blk->used)
    {
        ERROR("Double free on block %p\n", blk);
        ASSERT(0);
    }

    if (blk->previous) ASSERT(next_block(blk->previous) == blk);

    block_group* bg = blk->block_group;
    ASSERT(bg);
    ASSERT(bg->magic == BLOCK_GROUP_MAGIC);

    blk->used = 0;
    bg->largest_free_block = MAX(bg->largest_free_block, blk->size);

    int left_bordered = blk->previous == NULL;
    int right_bordered = next_block(blk) == NULL;

    if (left_bordered && right_bordered)
    {
        /* Nothing to merge */
    }
    else if (left_bordered && !right_bordered)
    {
        block* next = next_block(blk);
        if (next && !next->used)
        {
            merge_left(blk);
        }
    }
    else if (right_bordered && !left_bordered)
    {
        if (!blk->previous->used)
        {
            merge_right(blk);
        }
    }
    else if (!right_bordered && !left_bordered)
    {
        block* next = next_block(blk);
        if (!next->used && !blk->previous->used)
        {
            merge_middle(blk);
        }
    }

    if (used_block_count(bg) == 0)
    {
        /* all the blocks are free, erase the block group */
        delete_block_group(bg);
    }
}

void merge_left(block* target_block)
{
    block* free_block = next_block(target_block);
    ASSERT(!free_block->used);

    /* 'eat' the free block and mark the resulting block as free */
    target_block->size += free_block->size;
    target_block->block_group->largest_free_block =
            MAX(target_block->block_group->largest_free_block,
                target_block->size);

    if (next_block(target_block))
        next_block(target_block)->previous = target_block;

    ASSERT(target_block->magic == BLOCK_MAGIC);
}

void merge_right(block* target_block)
{
    ASSERT(target_block->previous);

    /* 'eat' the used block and mark the resulting block as free */
    target_block->previous->size += target_block->size;
    target_block->block_group->largest_free_block =
            MAX(target_block->block_group->largest_free_block,
                target_block->previous->size);

    ASSERT(target_block->previous->magic == BLOCK_MAGIC);
}

void merge_middle(block* target_block)
{
    ASSERT(target_block->previous && !target_block->previous->used);
    ASSERT(next_block(target_block) && !next_block(target_block)->used);

    block* old_next = next_block(next_block(target_block));

    ASSERT(next_block(target_block->previous) == target_block);

    target_block->previous->size +=
            target_block->size + next_block(target_block)->size;
    target_block->block_group->largest_free_block =
            MAX(target_block->block_group->largest_free_block,
                target_block->previous->size);

    block* new_next = next_block(target_block->previous);
    if (new_next) new_next->previous = target_block->previous;
    ASSERT(new_next == old_next);

    ASSERT(target_block->previous->magic == BLOCK_MAGIC);
}
