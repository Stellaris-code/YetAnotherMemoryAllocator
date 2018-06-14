/*
log.h

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
#ifndef YAM_LOG_H
#define YAM_LOG_H

#if !defined(NDEBUG) && !defined(YAM_RELEASE)
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#endif

#if !defined(NDEBUG) && !defined(YAM_RELEASE)
#define LOG(fmt, ...) printf("YAM (log) : " fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

#if !defined(NDEBUG) && !defined(YAM_RELEASE)
#define ERROR(fmt, ...) printf("YAM (error) : " fmt, ##__VA_ARGS__)
#else
#define ERROR(fmt, ...)
#endif

#if !defined(NDEBUG) && !defined(YAM_RELEASE)
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void dump_yam_state(void);

#ifdef __cplusplus
}
#endif

#endif /* YAM_LOG_H */
