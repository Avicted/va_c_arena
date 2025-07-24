# va_c_arena

va_c_arena is a C99/C11 arena memory management library designed to provide efficient memory allocation and deallocation. This library is structured as a single-header, stb-style implementation for easy integration and cross-platform use.

> **Note:**  
> - The core arena allocator is fully C99-compliant.
> - **Aligned allocation (`arena_alloc_aligned`) and the provided tests/examples require C11 or newer** for features like `_Alignas`, `_Alignof`, and their macros (`VA_ALIGNAS`, `VA_ALIGNOF`).  
> - Compilation with `-std=c99` will fail if you use aligned allocation or the aligned examples/tests.  
> - Compilation with `-std=c11`, `-std=c17`, or `-std=c23` is fully supported.

## Features

- **Arena Memory Management**: Efficiently allocate and manage memory in blocks.
- **Single-Header (stb-style)**: Just include one header and add `#define VA_ARENA_IMPLEMENTATION` in one source file.
- **Cross-Platform**: Works on various operating systems without modification.
- **Easy to Use API**: Simple functions for memory management that are easy to integrate into your projects.
- **Unit Tested**: Includes tests for all API functions.
- **Aligned Allocations**: Supports aligned allocations for types with strict alignment requirements (C11 or newer required).

## Requirements

- **C99 compiler** for the core arena allocator.
- **C11 or newer compiler** for aligned allocations and tests/examples using `VA_ALIGNAS`/`VA_ALIGNOF`.
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

## Alignment and Thread Safety

- **Alignment:** `arena_alloc` returns memory aligned at least to `alignof(max_align_t)` (C11+) or `_Alignof(max_align_t)` (C11/C17). For stricter alignment, use `arena_alloc_aligned` (requires C11 or newer).
- **Thread Safety:** This arena implementation is **not thread-safe**. If you need to use an arena from multiple threads, you must provide your own synchronization.

## Building

A sample `build.sh` is provided. To build all examples and tests:

```sh
./build.sh
```

To test with different C standards (and verify alignment macro support):

```sh
./build.sh test-stds
```

To run an example or test:

```sh
./build.sh example         # Run the basic example
./build.sh aligned         # Run the aligned allocation example
./build.sh tests           # Run the unit tests
```

## Running Valgrind

To check all executables for memory leaks with Valgrind:

```sh
./build.sh valgrind
```

## License

This project is licensed under the MIT License.