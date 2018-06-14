#include <gtest/gtest.h>

#include "yam.h"

namespace
{

TEST(YamInterface, Alloc)
{
    {
        yam_used_memory = yam_allocated_memory = 0;

        char* data = (char*)yam_alloc(33);
        for (size_t i { 0 }; i < 32; ++i)
        {
            data[i] = 'a' + (i%26);
        }
        data[32] = '\0';

        EXPECT_STREQ(data, "abcdefghijklmnopqrstuvwxyzabcdef");

        printf("[          ] allocated memory = %d (0x%x)\n", yam_allocated_memory, yam_allocated_memory);
    }

    {
        yam_used_memory = yam_allocated_memory = 0;

        uint8_t* data = (uint8_t*)yam_alloc(0x2000);
        for (size_t i { 0 }; i < 0x2000; ++i)
        {
            data[i] = 0xFF;
        }
        bool result = true;
        for (size_t i { 0 }; i < 0x2000; ++i)
        {
            if (data[i] != 0xFF)
            {
                result = false;
                break;
            }
        }

        EXPECT_TRUE(result);
        printf("[          ] allocated memory = %d (0x%x)\n", yam_allocated_memory, yam_allocated_memory);
    }
}

TEST(YamInterface, Free)
{
    {
        yam_used_memory = yam_allocated_memory = 0;

        char* data = (char*)yam_alloc(33);
        EXPECT_NO_FATAL_FAILURE(yam_free(data));

        EXPECT_EQ(yam_used_memory, 0);
        EXPECT_EQ(yam_allocated_memory, 0);
    }

    {
        yam_used_memory = yam_allocated_memory = 0;

        uint8_t* data = (uint8_t*)yam_alloc(0x2000);
        EXPECT_NO_FATAL_FAILURE(yam_free(data));

        EXPECT_EQ(yam_used_memory, 0);
        EXPECT_EQ(yam_allocated_memory, 0);
    }
}

}
