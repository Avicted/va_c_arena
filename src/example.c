#include <stdio.h>

#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"

// Define macros for memory size conversions
#define KB(x) ((x) * 1024)
#define MB(x) ((x) * 1024 * 1024)

#define ARENA_SIZE MB(16)

int main(void)
{
    printf("va_c_arena - Arena Memory Management Example\n\n");

    Arena *arena = arena_create(ARENA_SIZE);
    if (!arena)
    {
        fprintf(stderr, "Failed to create arena\n");
        return 1;
    }

    void *ptr1 = arena_alloc(arena, (ARENA_SIZE / 2.0));
    if (!ptr1)
    {
        fprintf(stderr, "Failed to allocate memory from arena\n");
        arena_destroy(&arena);
        return 1;
    }

    snprintf((char *)ptr1, 256, "Hello from arena memory!");
    printf("%s\n", (char *)ptr1);

    void *ptr2 = arena_alloc(arena, (ARENA_SIZE / 2.0));
    if (!ptr2)
    {
        fprintf(stderr, "Failed to allocate memory from arena\n");
        arena_destroy(&arena);
        return 1;
    }

    printf("Arena created and memory allocated successfully.\n");
    printf("Total arena size:\t\t%zu bytes\n", arena_total_size(arena));
    printf("Memory used in arena:\t\t%zu bytes\n", arena_used_size(arena));
    printf("Memory available in arena:\t%zu bytes\n", arena_total_size(arena) - arena_used_size(arena));

    arena_destroy(&arena);

    if (arena_used_size(arena) != 0)
    {
        fprintf(stderr, "Arena was not destroyed properly, memory still used.\n");
        return 1;
    }

    printf("\nArena destroyed successfully.\n");

    return 0;
}
