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

    // Create an arena with a size of 32 MB
    Arena *arena = arena_create(ARENA_SIZE);
    if (!arena)
    {
        fprintf(stderr, "Failed to create arena\n");
        return 1;
    }

    // Allocate memory from the arena
    void *ptr1 = arena_alloc(arena, (ARENA_SIZE / 2.0));
    if (!ptr1)
    {
        fprintf(stderr, "Failed to allocate memory from arena\n");
        arena_destroy(&arena);
        return 1;
    }

    // Use the allocated memory (example)
    snprintf((char *)ptr1, 256, "Hello from arena memory!");
    printf("%s\n", (char *)ptr1);

    // Allocate more memory
    void *ptr2 = arena_alloc(arena, (ARENA_SIZE / 2.0));
    if (!ptr2)
    {
        fprintf(stderr, "Failed to allocate memory from arena\n");
        arena_destroy(&arena);
        return 1;
    }

    // Assert that the arena was created and memory was allocated successfully
    printf("Arena created and memory allocated successfully.\n");
    printf("Total arena size:\t\t%zu bytes\n", arena_total_size(arena));
    printf("Memory used in arena:\t\t%zu bytes\n", arena_used(arena));
    printf("Memory available in arena:\t%zu bytes\n", arena_total_size(arena) - arena_used(arena));

    // Clean up
    arena_destroy(&arena);

    // Assert that the arena was destroyed successfully
    if (arena_used(arena) != 0)
    {
        fprintf(stderr, "Arena was not destroyed properly, memory still used.\n");
        return 1;
    }

    printf("\nArena destroyed successfully.\n");

    return 0;
}
