/*
liballoc_tests.cpp

Copyright (c) 13 Yann BOUCHER (yann)

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

// Source : https://github.com/blanham/liballoc/blob/master/test/malloc_test.cpp

#include <gtest/gtest.h>

#include <stdio.h>
#include <limits.h>
#include <time.h>

#include "yam.h"
#include "internal/debug.h"
#include "internal/block_record.h"

#if 1
    #define MAX_BLOCKS			100
    #define MAX_SIZE			(1024 * 1024)
    #define MAX_TIME			( 1 * 60 )
#else
    #define MAX_BLOCKS			2
    #define MAX_SIZE			(128)
    #define MAX_TIME			(1)
#endif

namespace
{

/** A testing testing_block to hold all allocated data. */
struct testing_block
{
    unsigned char *data;
    int size;
    unsigned char key;
};



/** The testing testing_blocks. */
static struct testing_block testing_blocks[ MAX_BLOCKS ];
static long long totalMemory = 0;
static int totalBlocks = 0;


static int g_verbose = 0;




static int malloc_random( int verbose )
{
    g_verbose = verbose;
    totalMemory = 0;
    totalBlocks = 0;

    printf("malloc_random: this will take %i minute...\n", MAX_TIME/ 60 );


    for ( int i = 0; i < MAX_BLOCKS; i++ )
    {
        testing_blocks[ i ].data = NULL;
        testing_blocks[ i ].size = 0;
        testing_blocks[ i ].key  = 0;
    }

    int transactions = 0;
    time_t start_time = time(NULL);

    //	Random madness.
    while (1==1)
    {
        int position = rand() % MAX_BLOCKS;

        int diff = time(NULL) - start_time;
        if ( diff > ( MAX_TIME ) ) break;

        int tps = (++transactions) / (diff + 1);



          if ( testing_blocks[position].data == NULL )
          {
            testing_blocks[position].size = rand() % MAX_SIZE;
            testing_blocks[position].data = (unsigned char*)yam_alloc( testing_blocks[position].size );
            testing_blocks[position].key  = rand() % 256;

            if ( g_verbose != 0 )
                printf("%i left, %i tps : %i, %i : %i: allocating %i bytes with %x key\n",
                                ( MAX_TIME - diff ),
                                tps,
                                totalBlocks * 100 / MAX_BLOCKS,
                                (int)(totalMemory / (1024)),
                                position,
                                testing_blocks[position].size,
                                testing_blocks[position].key );

            if ( testing_blocks[position].data != NULL )
            {
                totalMemory += testing_blocks[position].size;
                totalBlocks += 1;

                for ( int j = 0; j < testing_blocks[position].size; j++ )
                    testing_blocks[position].data[j] = testing_blocks[position].key;
            }

          }
          else
          {
                for ( int j = 0; j < testing_blocks[position].size; j++ )
                    if ( testing_blocks[position].data[j] != testing_blocks[position].key )
                    {
                        printf( "%i: %p (%i bytes, position %i) %i != %i: ERROR! Memory not consistent",
                                        position,
                                        testing_blocks[position].data,
                                        testing_blocks[position].size,
                                        j,
                                        testing_blocks[position].data[j],
                                        testing_blocks[position].key );
                        block* ptr = *((block**)(testing_blocks[position].data-8));
                        dump_yam_state();
                        fflush(stdout);

                        abort();
                    }


                if ( g_verbose != 0 )
                    printf("%i left, %i tps : %i, %i : %i: freeing %i bytes with %x key\n",
                                ( MAX_TIME - diff ),
                                tps,
                                totalBlocks * 100 / MAX_BLOCKS,
                                (int)(totalMemory / (1024)),
                                position,
                                testing_blocks[position].size,
                                testing_blocks[position].key );

                yam_free( testing_blocks[position].data );
                testing_blocks[position].data = NULL;

                totalMemory -= testing_blocks[position].size;
                totalBlocks -= 1;
          }

    }

    // Dump the memory map here.


    // Free.
    for ( int i = 0; i < MAX_BLOCKS; i++ )
    {
        if ( testing_blocks[ i ].data != NULL ) yam_free( testing_blocks[ i ].data );
        testing_blocks[ i ].size = 0;
        testing_blocks[ i ].key  = 0;
    }


    // Final results.
    printf("%i TPS, %i%s USAGE\n", transactions / MAX_TIME, totalBlocks * 100 / MAX_BLOCKS, "%" );

    return 0;
}




static int malloc_large( int verbose )
{
    g_verbose = verbose;

    printf("malloc_large: going to exhaust the memory...\n" );

    for ( int i = 0; i < MAX_BLOCKS; i++ )
        testing_blocks[ i ].data = NULL;

    int transactions = 0;
    time_t start_time = time(NULL);

    for ( int i = 0; i < MAX_BLOCKS; i++ )
    {
        testing_blocks[ i ].data = (unsigned char*)yam_alloc( MAX_SIZE );
        if ( testing_blocks[i].data == NULL ) break;

        transactions += 1;
    }

    for ( int i = 0; i < MAX_BLOCKS; i++ )
        if ( testing_blocks[ i ].data != NULL ) yam_free( testing_blocks[ i ].data );


    // Final results.
    printf("%i testing_blocks of %i size = %i MB, %i seconds\n",
             transactions,
             MAX_SIZE,
             (transactions * MAX_SIZE) / (1024 * 1024),
             time(NULL) - start_time
             );

    return 0;
}



int malloc_test( int verbose )
{
    malloc_large( verbose );
    malloc_large( verbose );
    malloc_large( verbose );
    malloc_random( verbose );
    malloc_random( verbose );
    malloc_random( verbose );

    return 0;
}

TEST(ExternalTests, LibAlloc)
{
    EXPECT_NO_FATAL_FAILURE(malloc_test(1));
}

}
