# va_c_arena

va_c_arena is a C99/C11 arena memory management library designed to provide efficient memory allocation and deallocation. This library is structured as a single-header, stb-style implementation for easy integration and cross-platform use.

> **Note:**  
> - The core arena allocator is fully C99-compliant.
> - **Aligned allocation (`arena_alloc_aligned`) and the provided tests/examples require C11 or newer** for features like `_Alignas`, `_Alignof`, and their macros (`VA_ALIGNAS`, `VA_ALIGNOF`).  
> - Compilation with `-std=c99` will fail if you use aligned allocation or the aligned examples/tests.  
> - Compilation with `-std=c11`, `-std=c17`, or `-std=c23` is fully supported.

## Features

- **Arena Memory Management**: Efficiently allocate and manage memory in blocks.
- **Single-Header (stb-style)**: Just include one header `include/va_arena.h` and add `#define VA_ARENA_IMPLEMENTATION` in one source file.
- **Cross-Platform**: Works on various operating systems without modification.
- **Portable:** `arena_create` uses only standard C (`malloc`, `realloc`, `free`). Reserved arenas use `mmap` (POSIX) or `VirtualAlloc` (Windows); define `VA_ARENA_NO_VM` to compile them out.
- **Reserved arenas (0.4):** `arena_create_reserve` reserves address space and commits pages on demand. Memory never moves, so pointers stay valid while the arena grows.
- **Temp scopes (0.4):** `arena_temp_begin` / `arena_temp_end` (and `arena_pos` / `arena_pop_to`) free everything allocated since a saved position.
- **In-place growth (0.4):** `arena_realloc` grows the most recent allocation in place, so string builders and dynamic arrays don't copy.
- **Helpers (0.4):** `arena_alloc_zero`, the overflow-checked `VA_PUSH_ARRAY` / `VA_PUSH_ARRAY_ZERO` / `VA_PUSH_STRUCT`, and `arena_strdup`, `arena_strndup`, `arena_printf`.
- **AddressSanitizer aware (0.4):** under ASan, unused and popped arena memory is poisoned.
- **Easy to Use API**: Simple functions for memory management that are easy to integrate into your projects.
- **Unit Tested**: Includes tests for all API functions.
- **Aligned Allocations**: Supports aligned allocations for types with strict alignment requirements (C11 or newer required).
- **Expandable Arena**: Optionally grow the arena at runtime with `arena_expand`.

## Requirements

- **C99 compiler** for the core arena functionality.
- **C11 or newer compiler** only for aligned allocations and tests/examples using `VA_ALIGNAS`/`VA_ALIGNOF`.
- Standard C library.

## Example (Aligned Allocation, C11+)

```c
#include <stdio.h>
#include <stdalign.h>

#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"

typedef struct {
    VA_ALIGNAS(32) int id;
    VA_ALIGNAS(32) double position[3];
    VA_ALIGNAS(32) char name[32];
} Entity;

int main(void) {
    Arena *arena = arena_create(4096);
    Entity *entities = (Entity *)arena_alloc_aligned(arena, sizeof(Entity) * 10, 32);
    if (!entities) {
        fprintf(stderr, "Failed to allocate aligned entities\n");
        arena_destroy(&arena);
        return 1;
    }
    // Use entities...
    arena_destroy(&arena);
    return 0;
}
```

## Reserved arenas and temp scopes (0.4)

```c
#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"   // first include of the implementation file (see below)

int main(void) {
    // 64 GiB of address space; only the pages actually used are committed
    Arena *perm = arena_create_reserve((size_t)64 << 30, 0);

    char *name = arena_printf(perm, "%s-%d", "hook", 42);   // lives as long as perm

    ArenaTemp t = arena_temp_begin(perm);
    int *scratch = VA_PUSH_ARRAY(perm, int, 1000000);        // overflow-checked count * size
    // ... use scratch ...
    arena_temp_end(t);                                       // scratch is gone, name is kept

    arena_destroy(&perm);
    return name != NULL ? 0 : 1;
}
```

Under strict `-std=c99`/`-std=c11`, include `va_arena.h` first in the implementation file so it can expose `MAP_ANONYMOUS`; otherwise it falls back to mapping `/dev/zero`.

## Alignment and Thread Safety

- **Alignment:** `arena_alloc` returns memory aligned at least to `alignof(max_align_t)` (C11+) or `_Alignof(max_align_t)` (C11/C17). For stricter alignment, use `arena_alloc_aligned` (requires C11 or newer).
- **Thread Safety:** This arena implementation is **not thread-safe**. If you need to use an arena from multiple threads, you must provide your own synchronization.


## Arena Expansion and Pointer Invalidation

If you use `arena_expand` to grow the arena, **all pointers previously returned by `arena_alloc` or `arena_alloc_aligned` may become invalid**. This is because the underlying memory block may be moved by `realloc`.  
**After calling `arena_expand`, you must not use any pointers previously obtained from the arena.**

Example:
```c
void *ptr = arena_alloc(arena, 128);
// ... use ptr ...
arena_expand(arena, 4096);
// DO NOT use ptr here! It may point to invalid memory.
```

## Building

Build with `make` (requires CMake 3.16+):

```sh
make          # Build all examples and tests
```

Or use CMake directly for IDE integration:

```sh
cmake -B build -S .
cmake --build build
```

A `compile_commands.json` is generated inside `build/` for clangd/LSP support.

### Run targets

```sh
make example    # Run the basic example
make aligned    # Run the aligned allocation example
make test       # Run the unit tests
```

### Check targets

```sh
make valgrind   # Valgrind leak check on all executables
make test-stds  # Build example_aligned under c99, c11, c17, c23
make asan       # Unit tests under AddressSanitizer + UBSan (also checks the poisoning)
```

### Other

```sh
make clean      # Remove build directory
make help       # Show all available targets
CC=clang make   # Build with a specific C compiler (default: clang)
```

--- 

## License

This project is licensed under the MIT License.
