#ifndef VA_ARENA_H
#define VA_ARENA_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

//    va_arena.h - stb-style C99/C11 arena allocator
//
//    Version: 0.2.0
//
//
//    Alignment Guarantees:
//    - Memory returned by arena_alloc() is aligned at least to alignof(max_align_t)
//      if the arena itself is allocated by malloc.
//
//    - For stricter alignment, use arena_alloc_aligned(), which allows you to specify
//      the alignment (must be a power of two).
//
//    - If you require allocations for types with alignment greater than
//      alignof(max_align_t), always use arena_alloc_aligned.
//
//    Thread Safety:
//    - This arena implementation is NOT thread-safe.
//    - If you need to use an arena from multiple threads, you must provide your own synchronization.

// Standard Alignment Macros:
// VA_ALIGNOF: Determines the alignment requirement of a type.
// C11/C17: _Alignof
// C23:     alignof
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define VA_ALIGNOF(type) alignof(type)
#else
#define VA_ALIGNOF(type) _Alignof(type)
#endif

// VA_ALIGNAS: Specifies the alignment requirement for a variable or type.
// C11/C17: _Alignas
// C23:      alignas
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define VA_ALIGNAS(x) alignas(x)
#else
#define VA_ALIGNAS(x) _Alignas(x)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Arena
    {
        size_t size;
        size_t used;
        uint8_t *memory;
    } Arena;

    // Initializes an arena with the specified size
    Arena *arena_create(size_t size);

    // Destroys the arena and frees its memory
    void arena_destroy(Arena **arena);

    // Allocates memory from the arena
    void *arena_alloc(Arena *arena, size_t size);

    // Allocates memory from the arena with specified alignment
    void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);

    // Increases the arena size to accommodate more memory.
    // WARNING: All pointers previously returned by arena_alloc/arena_alloc_aligned may become invalid!
    // Returns a pointer to the current position in the new memory block, or NULL on failure.
    void *arena_expand(Arena *arena, size_t new_size);

    // Resets the arena, freeing all allocated memory
    int arena_reset(Arena *arena);

    // Returns the amount of memory currently used in the arena
    size_t arena_used_size(Arena *arena);

    // Returns the total size of the arena
    size_t arena_total_size(Arena *arena);

#ifdef __cplusplus
}
#endif

#endif // VA_ARENA_H

// ================== IMPLEMENTATION ==================
#ifdef VA_ARENA_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

Arena *arena_create(size_t size)
{
    Arena *arena = (Arena *)malloc(sizeof(Arena));
    if (!arena)
    {
        return NULL;
    }

    arena->memory = (uint8_t *)malloc(size);
    if (!arena->memory)
    {
        free(arena);
        return NULL;
    }

    arena->size = size;
    arena->used = 0;

    return arena;
}

void arena_destroy(Arena **arena)
{
    if (arena && *arena)
    {
        free((*arena)->memory);
        (*arena)->memory = NULL;
        (*arena)->used = 0;
        free(*arena);
        *arena = NULL;
    }
}

void *arena_alloc(Arena *arena, size_t size)
{
    if (!arena || size == 0)
    {
        return NULL;
    }

    if (arena->used + size > arena->size)
    {
        return NULL;
    }

    void *ptr = arena->memory + arena->used;
    arena->used += size;

    return ptr;
}

size_t align_up(size_t n, size_t align)
{
    // Assumes alignment is a power of two.
    // The & ~(align - 1) operation rounds n up to the nearest multiple of align.
    return (n + align - 1) & ~(align - 1);
}

void *arena_alloc_aligned(Arena *arena, size_t size, size_t alignment)
{
    size_t current = (size_t)(arena->memory + arena->used);
    size_t aligned = align_up(current, alignment);
    size_t offset = aligned - (size_t)arena->memory;

    if (offset + size > arena->size)
    {
        return NULL;
    }

    void *ptr = arena->memory + offset;
    arena->used = offset + size;

    return ptr;
}

void *arena_expand(Arena *arena, size_t new_size)
{
    if (!arena || new_size <= arena->size)
    {
        return NULL;
    }

    uint8_t *new_memory = (uint8_t *)realloc(arena->memory, new_size);
    if (!new_memory)
    {
        return NULL; // Reallocation failed
    }

    arena->memory = new_memory;
    arena->size = new_size;

    return arena->memory + arena->used;
}

int arena_reset(Arena *arena)
{
    if (!arena || !arena->memory)
    {
        return -1;
    }

    arena->used = 0;

    return 0;
}

size_t arena_used_size(Arena *arena)
{
    if (!arena)
    {
        return 0;
    }

    return arena->used;
}

size_t arena_total_size(Arena *arena)
{
    if (!arena)
    {
        return 0;
    }

    return arena->size;
}

#endif // VA_ARENA_IMPLEMENTATION
