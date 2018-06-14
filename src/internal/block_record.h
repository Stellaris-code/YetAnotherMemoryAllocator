/*
block_record.h

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
#ifndef BLOCK_RECORD_H
#define BLOCK_RECORD_H

#include <stdint.h>
#include <stddef.h>

#define BLOCK_MAGIC 0xF00D

typedef struct /*__attribute__((packed))*/ block
{
    struct block*       previous;
    struct block_group* block_group;

#if 0
    uint32_t size;
    int      used;
#else
    uint32_t size : 31;
    int      used : 1;
#endif

    uint16_t magic;

    uint8_t data[];
} block;

#ifdef __cplusplus
extern "C"
{
#endif

block* next_block(const block* in_block);

#ifdef __cplusplus
}
#endif

#endif /* BLOCK_RECORD_H */
