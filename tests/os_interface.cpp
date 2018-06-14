#include <gtest/gtest.h>

#include "internal/block_record.h"

#define SENTINEL_PAGES 2

extern "C" uintptr_t os_alloc_pages(unsigned count);
extern "C" int       os_free_pages (uintptr_t base, unsigned count);
extern "C" void      dump_yam_state(void);
extern "C" block* allocate_new_block(size_t size);

extern "C" void      init();

namespace
{

TEST(OsInterfacePage, Alloc)
{
    init();

    {
        uintptr_t res = os_alloc_pages(1);
        EXPECT_TRUE(res != (uintptr_t)-1);

#if SENTINEL_PAGES && 0
        EXPECT_EXIT(((volatile unsigned char*)res)[1*sysconf(_SC_PAGESIZE)] = 0xFF, ::testing::KilledBySignal(SIGSEGV), ".*");
#endif
    }
    {
        uintptr_t res = os_alloc_pages(4);
        EXPECT_TRUE(res != (uintptr_t)-1);
        for (int i { 0 }; i < 4*sysconf(_SC_PAGESIZE); ++i)
        {
            // Test that pages are accessible and readable/writable
            ((volatile unsigned char*)res)[i] = 0xFF;
            EXPECT_EQ(((volatile unsigned char*)res)[i], 0xFF);
        }
#if SENTINEL_PAGES && 0
        EXPECT_EXIT(((volatile unsigned char*)res)[4*sysconf(_SC_PAGESIZE)] = 0xFF, ::testing::KilledBySignal(SIGSEGV), ".*");
#endif
    }
}

TEST(OsInterfacePage, Free)
{
    {
        uintptr_t res = os_alloc_pages(1);
        EXPECT_EQ(os_free_pages(res, 1), 0);
    }
    {
        uintptr_t res = os_alloc_pages(64);
        EXPECT_EQ(os_free_pages(res, 64), 0);
    }
    {
        uintptr_t res = os_alloc_pages(2);
        EXPECT_EQ(os_free_pages(res, 2), 0);
    }
}

}
