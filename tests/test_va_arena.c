#include <stdalign.h>
#include <stdio.h>

#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"

int test_arena_create(void)
{
    Arena *arena = arena_create(1024);
    if (arena == NULL)
    {
        printf("test_arena_create\t\t\tfailed\n");
        return -1;
    }
    else
    {
        printf("test_arena_create\t\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_alloc(void)
{
    Arena *arena = arena_create(1024);
    void *ptr = arena_alloc(arena, 128);
    if (ptr == NULL)
    {
        printf("test_arena_alloc\t\t\tfailed\n");
        return -1;
    }
    else
    {
        printf("test_arena_alloc\t\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_alloc_exceed(void)
{
    Arena *arena = arena_create(1024);
    void *ptr = arena_alloc(arena, 2048);

    if (ptr != NULL)
    {
        printf("test_arena_alloc_exceed\t\t\tfailed\n");
        return -1;
    }
    else
    {
        printf("test_arena_alloc_exceed\t\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_alloc_aligned(void)
{
    Arena *arena = arena_create(1024);
    void *ptr = arena_alloc_aligned(arena, 64, VA_ALIGNOF(long double));
    if (ptr == NULL)
    {
        printf("test_arena_alloc_aligned\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else if (((uintptr_t)ptr) % VA_ALIGNOF(double) != 0)
    {
        printf("test_arena_alloc_aligned\t\tfailed\n");
        return -1;
        arena_destroy(&arena);
    }
    else
    {
        printf("test_arena_alloc_aligned\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_alloc_aligned_exceed(void)
{
    Arena *arena = arena_create(1024);
    void *ptr = arena_alloc_aligned(arena, 2048, VA_ALIGNOF(double));
    if (ptr != NULL)
    {
        printf("test_arena_alloc_aligned_exceed\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else
    {
        printf("test_arena_alloc_aligned_exceed\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_destroy(void)
{
    Arena *arena = arena_create(1024);
    arena_destroy(&arena);
    printf("test_arena_destroy\t\t\tpassed\n");

    if (arena != NULL)
    {
        printf("test_arena_destroy\t\t\tfailed\n");
        return -1;
    }

    return 0;
}

int test_arena_reset(void)
{
    Arena *arena = arena_create(1024);
    void *ptr = arena_alloc(arena, 512);

    if (!ptr)
    {
        printf("test_arena_reset\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }

    int reset_result = arena_reset(arena);
    if (reset_result != 0)
    {
        printf("test_arena_reset\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else if (arena_used_size(arena) != 0)
    {
        printf("test_arena_reset\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else
    {
        printf("test_arena_reset\t\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_used_and_total_size(void)
{
    Arena *arena = arena_create(1024);
    if (!arena)
    {
        printf("test_arena_used_and_total_size\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    if (arena_total_size(arena) != 1024)
    {
        printf("test_arena_used_and_total_size\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    arena_alloc(arena, 128);
    if (arena_used_size(arena) != 128)
    {
        printf("test_arena_used_and_total_size\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else
    {
        printf("test_arena_used_and_total_size\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int test_arena_alignment(void)
{
    Arena *arena = arena_create(1024);
    if (!arena)
    {
        printf("test_arena_alignment\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }

    void *ptr = arena_alloc(arena, sizeof(double));
    if (!ptr)
    {
        printf("test_arena_alignment\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    if (((uintptr_t)ptr) % VA_ALIGNOF(double) != 0)
    {
        printf("test_arena_alignment\t\t\tfailed\n");
        arena_destroy(&arena);
        return -1;
    }
    else
    {
        printf("test_arena_alignment\t\t\tpassed\n");
        arena_destroy(&arena);
        return 0;
    }
}

int main(void)
{
    printf("va_c_arena - Arena Memory Management Tests\n\n");

    size_t success_count = 0;
    size_t error_count = 0;

    int (*tests[])(void) = {test_arena_create,
                            test_arena_alloc,
                            test_arena_alloc_exceed,
                            test_arena_alloc_aligned,
                            test_arena_alloc_aligned_exceed,
                            test_arena_destroy,
                            test_arena_reset,
                            test_arena_used_and_total_size,
                            test_arena_alignment};

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
        if (tests[i]() == 0)
        {
            success_count++;
        }
        else
        {
            error_count++;
        }
    }

    printf("\nTests completed: %zu passed, %zu failed.\n", success_count, error_count);
    return 0;
}
