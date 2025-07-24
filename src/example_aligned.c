#include <stdio.h>

#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"

// Define macros for memory size conversions
#define KB(x) ((x) * 1024)

#define ENTITY_COUNT 512
#define ARENA_SIZE (ENTITY_COUNT * sizeof(Entity) + KB(1))

typedef struct
{
    VA_ALIGNAS(32) int id;
    VA_ALIGNAS(32) double position[3];
    VA_ALIGNAS(32) char name[32];
} Entity;

int main(void)
{
    printf("va_c_arena - Aligned Entity Allocation Example\n\n");

    Arena *arena = arena_create(ARENA_SIZE);
    if (!arena)
    {
        fprintf(stderr, "Failed to create arena\n");
        return 1;
    }

    // Allocate all entities at once, aligned to 32 bytes
    Entity *entities = (Entity *)arena_alloc_aligned(arena, sizeof(Entity) * ENTITY_COUNT, 32);
    if (!entities)
    {
        fprintf(stderr, "Failed to allocate aligned entities\n");
        arena_destroy(&arena);
        return 1;
    }

    // Initialize and test alignment
    int alignment_errors = 0;
    for (size_t i = 0; i < ENTITY_COUNT; ++i)
    {
        entities[i].id = (int)i;
        entities[i].position[0] = i * 1.0;
        entities[i].position[1] = i * 2.0;
        entities[i].position[2] = i * 3.0;
        snprintf(entities[i].name, sizeof(entities[i].name), "Entity_%zu", i);

        if (((uintptr_t)&entities[i]) % 32 != 0)
        {
            printf("Alignment error: Entity %zu is not 32-byte aligned!\n", i);
            alignment_errors++;
        }
    }

    // Iterate and print a few entities
    for (size_t i = 0; i < 5; ++i)
    {
        printf("Entity %d: pos=(%.1f, %.1f, %.1f),\tname=%s\n", entities[i].id, entities[i].position[0],
               entities[i].position[1], entities[i].position[2], entities[i].name);
    }

    if (alignment_errors == 0)
    {
        printf("\nAll entities are properly 32-byte aligned.\n\n");
    }
    else
    {
        printf("\n%d entities were not properly aligned!\n\n", alignment_errors);
    }

    // Assert that the arena was created and memory was allocated successfully
    printf("Arena created and memory allocated successfully.\n");
    printf("Total arena size:\t\t%zu bytes\n", arena_total_size(arena));
    printf("Memory used in arena:\t\t%zu bytes\n", arena_used(arena));
    printf("Memory available in arena:\t%zu bytes\n", arena_total_size(arena) - arena_used(arena));

    arena_destroy(&arena);

    // Assert that the arena was destroyed successfully
    if (arena_used(arena) != 0)
    {
        fprintf(stderr, "Arena was not destroyed properly, memory still used.\n");
        return 1;
    }

    printf("\nArena destroyed successfully.\n");

    return alignment_errors ? 1 : 0;
}
