/*
debug.c

Copyright (c) 10 Yann BOUCHER (yann)

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

#include "debug.h"

#include "block_group_record.h"

#include <stdio.h>
#include <inttypes.h>

extern block_group* first_block_group;
extern int          page_size;

void dump_yam_state()
{
    puts("------------------");

    block_group* group = first_block_group;

    while (group)
    {
        if (check_block_group(group))
            ERROR("Invalid block group magic\n");
        printf("Block at address [%p]\n", group);
        printf("Page  count : %u\n", group->page_count);
        printf("Largest free block : %u\n", group->largest_free_block);

        int block_counter = 1;
        block* current_block = group->first_block;
        while (current_block)
        {
            printf(" └╴Block #%d [%p-%p]:\n", block_counter, current_block, (uint8_t*)current_block + current_block->size);

            if (current_block->used)
                printf("    └╴Used block\n");
            else
                printf("    └╴Free block\n");

            printf("    └╴Block size : %" PRIu32 "\n", (uint32_t)current_block->size);

            if (!check_block(current_block))
            {
                printf("    └╴Block #%d has an invalid checksum !\n", block_counter);
                break;
            }

            current_block = next_block(current_block);

            ++block_counter;
        }

        group = group->next_block_group;
    }

    puts("------------------");
}
