/*
os_interface.c

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

#define _DEFAULT_SOURCE /* for MAP_ANONYMOUS */

#include "os_interface.h"

#include <stddef.h>

#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "debug.h"

#if SENTINEL_PAGES % 2
#error SENTINEL_PAGES must be a multiple of 2
#endif

int yam_allocated_pages = 0;

pthread_mutex_t yam_mutex = PTHREAD_MUTEX_INITIALIZER;

extern int page_size;

uintptr_t os_alloc_pages(unsigned count)
{
    void* result = mmap(NULL, page_size * (count + SENTINEL_PAGES),
                        PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED)
    {
        ERROR("Page allocation failure : %s (%d)\n", strerror(errno), errno);
        return (uintptr_t)-1;
    }

    /* Adjust the pointer according to the sentinel pages */
    result = (uint8_t*)result + page_size*(SENTINEL_PAGES/2);

#if SENTINEL_PAGES
    /* mark front and back pages as sentinels */
    mprotect((uint8_t*)result - page_size*(SENTINEL_PAGES/2),
             page_size*(SENTINEL_PAGES/2), PROT_NONE);
    mprotect((uint8_t*)result + page_size*count             ,
             page_size*(SENTINEL_PAGES/2), PROT_NONE);
#endif

    yam_allocated_pages += count;

    return (uintptr_t)result;
}

int os_free_pages (uintptr_t base, unsigned count)
{
    /* If page_size is invalid, no page should ever have been allocated */
    ASSERT(page_size);
    /* unmap sentinel page as well */
    int result = munmap((void*)(base - page_size*(SENTINEL_PAGES/2)),
                        (count + SENTINEL_PAGES)*page_size);

    if (result == -1)
    {
        ERROR("Page release failure : %s (%d)\n", strerror(errno), errno);
        return -1;
    }

    yam_allocated_pages -= count;

    return 0;
}

size_t os_get_page_size()
{
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size <= 0) /* What the hell is going on ?! */
    {
        ERROR("Can't retrieve system page size!\n");
        return (size_t)-1;
    }

    return page_size;
}

void os_lock()
{
    pthread_mutex_lock(&yam_mutex);
}

void os_unlock()
{
    pthread_mutex_unlock(&yam_mutex);
}
