/*
yam_stdlib.h

Copyright (c) 14 Yann BOUCHER (yann)

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
#ifndef YAM_STDLIB_H
#define YAM_STDLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef YAM_PREFIX
#define YAM_PREFIX(x) yam2_##x
#endif

void* YAM_PREFIX(malloc        )(size_t size);
void  YAM_PREFIX(free          )(void*  ptr);
void* YAM_PREFIX(calloc        )(size_t num, size_t size);
void* YAM_PREFIX(realloc       )(void*  ptr, size_t size);
void* YAM_PREFIX(aligned_alloc )(size_t alg, size_t size);
int   YAM_PREFIX(posix_memalign)(void** ptr, size_t alig, size_t size);

#ifdef __cplusplus
}
#endif


#endif // YAM_STDLIB_H
