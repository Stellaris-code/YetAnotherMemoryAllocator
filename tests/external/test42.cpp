#include <gtest/gtest.h>

#include "yam.h"
#include "internal/debug.h"
#include "internal/block_group_record.h"

extern "C" block_group* first_block_group;
extern "C" int          yam_page_allocation_factor;
extern "C" block_group* first_block_group;

namespace
{

#define M (1024 * 1024)

TEST(ExternalTests, Test42_1)
{
    first_block_group = nullptr;
    yam_allocated_pages = 0;

    int i;
    char *addr;

    i = 0;
    while (i < 1024)
    {
        addr = (char*)yam_aligned_alloc(1024, 1);
        addr[0] = 42;
        i++;
    }

    printf("Overhead : %d\n", sizeof(block)+sizeof(void*));
    printf("Reclaimed pages : %d\n", yam_allocated_pages);
    printf("aaaaa pages : %d\n", yam_page_allocation_factor);

    EXPECT_LT(yam_allocated_pages, 272);
}

TEST(ExternalTests, Test42_2)
{
    yam_allocated_pages = 0;
    yam_page_allocation_factor = 7;
    first_block_group = nullptr;

    int i;
    char *addr;

    i = 0;
    while (i < 1024)
    {
        addr = (char*)yam_alloc(1024);
        addr[0] = 42;
        yam_free(addr);
        i++;
    }

    EXPECT_LE(yam_allocated_pages, 3);
}

TEST(ExternalTests, Test42_3)
{
    yam_allocated_pages = 0;
    yam_page_allocation_factor = 7;
    first_block_group = nullptr;

    char output[256] = { 0 };

    char *addr1;
    char *addr3;

    addr1 = (char*)yam_alloc(16*M);
    strcpy(addr1, "Bonjours\n");
    strcat(output, addr1);
    addr3 = (char*)yam_realloc(addr1, 128*M);
    addr3[127*M] = 42;
    strcat(output, addr3);

    EXPECT_STREQ(output, "Bonjours\nBonjours\n");
}

TEST(ExternalTests, Test42_3plus)
{
    yam_allocated_pages = 0;
    yam_page_allocation_factor = 7;
    first_block_group = nullptr;

    char output[256] = { 0 };

    char *addr1;
    char *addr2;
    char *addr3;

    addr1 = (char*)yam_alloc(16*M);
    strcpy(addr1, "Bonjours\n");
    strcat(output, addr1);
    addr2 = (char*)yam_alloc(16*M);
    addr3 = (char*)yam_realloc(addr1, 128*M);
    addr3[127*M] = 42;
    strcat(output, addr3);

    EXPECT_STREQ(output, "Bonjours\nBonjours\n");
}

TEST(ExternalTests, Test42_4)
{
    yam_allocated_pages = 0;
    yam_page_allocation_factor = 7;
    first_block_group = nullptr;

    char *addr;

    addr = (char*)yam_alloc(16);
    yam_free(NULL);
    yam_free((uint8_t*)addr + 5);

    EXPECT_EQ(yam_realloc((uint8_t*)addr + 5, 10), nullptr);
}
}
