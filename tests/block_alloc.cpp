#include <gtest/gtest.h>

#include "internal/block_record.h"
#include "internal/block_group_record.h"

extern "C" uintptr_t    os_alloc_pages(unsigned count);
extern "C" int          os_free_pages (uintptr_t base, unsigned count);
extern "C" void         dump_yam_state(void);
extern "C" block_group* alloc_block_group(size_t byte_len);
extern "C" block*       allocate_new_block(size_t size);
extern "C" void         delete_block(block *block);
extern "C" void         init();

extern "C" block_group* first_block_group;
extern "C" int          page_size;
extern "C" int yam_page_allocation_factor;

int old_page_factor;

namespace
{

size_t block_group_count()
{
    size_t counter = 0;
    const block_group* current = first_block_group;
    while (current)
    {
        current = current->next_block_group;

        ++counter;
    }

    return counter;
}

const block_group* get_block_group(size_t idx)
{
    const block_group* current = first_block_group;
    while (current)
    {
        if (idx-- == 0)
        {
            return current;
        }

        current = current->next_block_group;
    }

    assert(false);
}

size_t block_count(const block_group* bg = first_block_group)
{
    const block* current_block = bg->first_block;
    size_t counter = 0;
    while (current_block)
    {
        ++counter;

        current_block = next_block(current_block);
    }

    return counter;
}

const block* get_block(size_t idx, const block_group* bg = first_block_group)
{
    const block* current_block = bg->first_block;

    while ((uintptr_t)current_block != (uintptr_t)bg + bg->page_count*page_size)
    {
        assert((uintptr_t)current_block < (uintptr_t)bg + bg->page_count*page_size);

        if (idx-- == 0)
        {
            return current_block;
        }

        current_block = next_block(current_block);
    }

    assert(false);
}

TEST(BlockAlloc, Alloc)
{
    init();
    old_page_factor = yam_page_allocation_factor;
    yam_page_allocation_factor = 1;

    {
        first_block_group = alloc_block_group(0x500);

        ASSERT_TRUE(allocate_new_block(256));

        EXPECT_EQ(block_count(), 2);
        EXPECT_EQ(get_block(0)->size, 256);
        EXPECT_TRUE(get_block(0)->used);

        EXPECT_EQ(get_block(1)->size, first_block_group->page_count*page_size-256-sizeof(block_group));
        EXPECT_FALSE(get_block(1)->used);

        EXPECT_EQ(first_block_group->largest_free_block, get_block(1)->size);
    }

    {
        first_block_group = alloc_block_group(0x1000-sizeof(block_group));

        ASSERT_TRUE(allocate_new_block(0x1000-sizeof(block_group)));

        EXPECT_EQ(block_count(), 1);
        EXPECT_EQ(get_block(0)->size, 0x1000-sizeof(block_group));
        EXPECT_TRUE(get_block(0)->used);

        EXPECT_EQ(first_block_group->largest_free_block, 0);
    }

    {
        first_block_group = alloc_block_group(0x1000-sizeof(block_group));

        ASSERT_TRUE(allocate_new_block(0x1800));
        EXPECT_EQ(block_group_count(), 2);

        EXPECT_EQ(block_count(get_block_group(0)), 1);
        EXPECT_FALSE(get_block(0, get_block_group(0))->used);
        EXPECT_EQ(block_count(get_block_group(1)), 2);
        EXPECT_TRUE(get_block(0, get_block_group(1))->used);

        EXPECT_EQ(get_block(0, get_block_group(1))->size, 0x1800);
        EXPECT_FALSE(get_block(1, get_block_group(1))->used);
        EXPECT_EQ(get_block(1, get_block_group(1))->size, 0x2000-0x1800-sizeof(block_group));

        EXPECT_EQ(get_block_group(0)->largest_free_block, 0x1000-sizeof(block_group));
        EXPECT_EQ(get_block_group(1)->largest_free_block, 0x2000-0x1800-sizeof(block_group));
    }

    yam_page_allocation_factor = old_page_factor;
}

TEST(BlockAlloc, Free)
{
    init();
    old_page_factor = yam_page_allocation_factor;
    yam_page_allocation_factor = 1;

    // Right
    {
        first_block_group = alloc_block_group(0x500);

        auto block_1 = allocate_new_block(60);
        auto block_2 = allocate_new_block(60);
        auto block_3 = allocate_new_block(page_size-120-sizeof(block_group));

        delete_block(block_2);
        delete_block(block_3);

        EXPECT_TRUE(block_1->used);
        EXPECT_FALSE(block_2->used);
        EXPECT_EQ((uintptr_t)block_2 + block_2->size, (uintptr_t)first_block_group + page_size);
    }

    // Full span
    {
        first_block_group = alloc_block_group(page_size-sizeof(block_group));

        auto block = allocate_new_block(page_size-sizeof(block_group));
        ASSERT_TRUE(block);

        delete_block(block);

        EXPECT_EQ(first_block_group, nullptr);
    }

    // Left
    {
        first_block_group = alloc_block_group(0x500);

        auto block = allocate_new_block(256);
        ASSERT_TRUE(block);

        delete_block(block);

        EXPECT_EQ(first_block_group, nullptr);
    }

    // Middle
    {
        first_block_group = alloc_block_group(0x500);

        auto block_1 = allocate_new_block(60);
        auto block_2 = allocate_new_block(60);
        auto block_3 = allocate_new_block(page_size-120-sizeof(block_group));

        delete_block(block_1);
        delete_block(block_3);

        delete_block(block_2);

        EXPECT_EQ(first_block_group, nullptr);
    }

    // None
    {
        first_block_group = alloc_block_group(0x500);

        auto block_1 = allocate_new_block(60);
        auto block_2 = allocate_new_block(60);
        auto block_3 = allocate_new_block(page_size-120-sizeof(block_group));

        delete_block(block_2);

        EXPECT_TRUE(block_1->used);
        EXPECT_TRUE(block_3->used);
        EXPECT_EQ((uintptr_t)block_3 + block_3->size, (uintptr_t)first_block_group + page_size);
    }

    yam_page_allocation_factor = old_page_factor;
}

TEST(BlockAlloc, BlockGroupDelete)
{
    init();
    old_page_factor = yam_page_allocation_factor;
    yam_page_allocation_factor = 1;
    // Back
    {
        first_block_group = alloc_block_group(page_size-sizeof(block_group));
        allocate_new_block(page_size-sizeof(block_group));

        auto block = allocate_new_block(page_size-sizeof(block_group));

        EXPECT_TRUE(first_block_group->next_block_group);

        delete_block(block);

        EXPECT_EQ(first_block_group->next_block_group, nullptr);
    }

    // Middle
    {
        first_block_group = alloc_block_group(page_size-sizeof(block_group));
        auto block_1 = allocate_new_block(page_size-sizeof(block_group)); (void)block_1;
        auto block_2 = allocate_new_block(page_size-sizeof(block_group));
        auto block_3 = allocate_new_block(page_size-sizeof(block_group)); (void)block_3;

        EXPECT_TRUE(first_block_group && first_block_group->next_block_group
                    && first_block_group->next_block_group->next_block_group);

        delete_block(block_2);

        EXPECT_TRUE(first_block_group && first_block_group->next_block_group);
        EXPECT_EQ(first_block_group->next_block_group->next_block_group, nullptr);
    }

    // Front
    {
        first_block_group = alloc_block_group(page_size-sizeof(block_group));
        auto block_1 = allocate_new_block(page_size-sizeof(block_group));
        auto block_2 = allocate_new_block(page_size-sizeof(block_group)); (void)block_2;

        EXPECT_TRUE(first_block_group && first_block_group->next_block_group);

        delete_block(block_1);

        EXPECT_TRUE(first_block_group);
        EXPECT_EQ(first_block_group->next_block_group, nullptr);
    }

    /* Check for cases when there isn't enough space for a new free block */
    {
        /* [UUUU....UU.UUU]
         *            ^
         *          too small */

        first_block_group = alloc_block_group(page_size-sizeof(block_group));
        auto block_3 = allocate_new_block(0xc00-sizeof(block_group)*3); (void)block_3;
        auto block_4 = allocate_new_block(0x400-sizeof(block_group));

        delete_block(block_4);

        auto block_5 = allocate_new_block(0x3f0-sizeof(block_group));

        EXPECT_EQ(block_5->size, 992);
    }

    yam_page_allocation_factor = old_page_factor;
}

}
