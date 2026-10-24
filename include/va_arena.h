// The VM backend uses POSIX mmap; under strict -std=c99/c11 this exposes MAP_ANONYMOUS when
// va_arena.h is the first include of the implementation file (otherwise /dev/zero is mapped
// instead). Define VA_ARENA_NO_VM to leave the OS out entirely (arena_create_reserve then fails).
#if defined(VA_ARENA_IMPLEMENTATION) && !defined(_WIN32) && !defined(VA_ARENA_NO_VM) && !defined(_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif

#ifndef VA_ARENA_H
#define VA_ARENA_H

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

//    va_arena.h - stb-style C99/C11 arena allocator
//
//    Version: 0.4.0
//
//
//    Two kinds of arena:
//
//    - arena_create(size): one malloc'd block of a fixed size. arena_expand() grows it with
//      realloc, which may MOVE the block: every pointer handed out before becomes invalid.
//
//    - arena_create_reserve(reserve, commit_step): reserves `reserve` bytes of address space
//      (mmap / VirtualAlloc; no memory is used yet) and commits pages on demand, commit_step at a
//      time, as allocations need them. The memory never moves, so pointers stay valid for the
//      arena's lifetime, and the most recent allocation can grow in place (arena_realloc). Reserve
//      generously (gigabytes on 64-bit): untouched address space costs nothing.
//
//    Freeing: there is no per-allocation free. Free everything with arena_reset/arena_destroy, or a
//    group of allocations with a temp scope:
//
//        ArenaTemp t = arena_temp_begin(arena);
//        ... allocations ...
//        arena_temp_end(t);           // everything since arena_temp_begin is gone
//
//    Zero-size requests: arena_alloc/arena_alloc_aligned return NULL for size 0 (as before). The
//    0.4 functions (arena_alloc_zero, arena_alloc_array, arena_realloc, arena_str*) return a valid
//    pointer for size 0, so NULL from them always means "out of memory".
//
//    Alignment Guarantees:
//    - Memory returned by arena_alloc() is aligned at least to alignof(max_align_t)
//      if the arena itself is allocated by malloc (reserved arenas: always).
//
//    - For stricter alignment, use arena_alloc_aligned(), which allows you to specify
//      the alignment (must be a power of two).
//
//    - If you require allocations for types with alignment greater than
//      alignof(max_align_t), always use arena_alloc_aligned.
//
//    AddressSanitizer: under ASan the unused tail of an arena (and anything popped by a temp scope
//    or reset) is poisoned, so stray accesses past an allocation into it are reported.
//
//    Thread Safety:
//    - This arena implementation is NOT thread-safe.
//    - If you need to use an arena from multiple threads, you must provide your
//    own synchronization (or give each thread its own arena).

// Standard Alignment Macros:
// VA_ALIGNOF: Determines the alignment requirement of a type.
// C11/C17: _Alignof
// C23:     alignof
// C99:     not defined (requires C11 or newer)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define VA_ALIGNOF(type) alignof(type)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define VA_ALIGNOF(type) _Alignof(type)
#endif

// VA_ALIGNAS: Specifies the alignment requirement for a variable or type.
// C11/C17: _Alignas
// C23:      alignas
// C99:     not defined (requires C11 or newer)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define VA_ALIGNAS(x) alignas(x)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define VA_ALIGNAS(x) _Alignas(x)
#endif

#if defined(__GNUC__) || defined(__clang__)
#if defined(__MINGW32__)
#define VA_PRINTF(fmt, args) __attribute__((format(gnu_printf, fmt, args)))
#else
#define VA_PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))
#endif
#else
#define VA_PRINTF(fmt, args)
#endif

// Typed pushes: VA_PUSH_ARRAY(arena, Type, count), overflow-checked count * sizeof(Type).
#ifdef VA_ALIGNOF
#define VA_PUSH_ARRAY(arena, type, count)	   ((type *)arena_alloc_array((arena), sizeof(type), (count), VA_ALIGNOF(type)))
#define VA_PUSH_ARRAY_ZERO(arena, type, count) ((type *)arena_alloc_array_zero((arena), sizeof(type), (count), VA_ALIGNOF(type)))
#else
#define VA_PUSH_ARRAY(arena, type, count)	   ((type *)arena_alloc_array((arena), sizeof(type), (count), 0))
#define VA_PUSH_ARRAY_ZERO(arena, type, count) ((type *)arena_alloc_array_zero((arena), sizeof(type), (count), 0))
#endif
#define VA_PUSH_STRUCT(arena, type)		 VA_PUSH_ARRAY((arena), type, 1)
#define VA_PUSH_STRUCT_ZERO(arena, type) VA_PUSH_ARRAY_ZERO((arena), type, 1)

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct Arena
	{
		size_t size; // usable bytes right now (committed bytes, for a reserved arena)
		size_t used; // offset of the first free byte
		uint8_t *memory;
		size_t reserved;	// reserved arenas: bytes of address space after `memory`; 0 = malloc'd
		size_t commit_step; // reserved arenas: commit granularity
		size_t last;		// offset of the most recent allocation (SIZE_MAX: none), for arena_realloc
	} Arena;

	// A saved position; arena_temp_end frees everything allocated after it.
	typedef struct ArenaTemp
	{
		Arena *arena;
		size_t pos;
		size_t last;
	} ArenaTemp;

	// Initializes an arena with the specified size
	Arena *
	arena_create(size_t size);

	// Reserves `reserve` bytes of address space and commits them `commit_step` bytes at a time,
	// on demand (rounded up to the page size; 0 = 64 KiB). Memory never moves. The Arena struct
	// itself lives at the start of the reservation: no malloc at all.
	Arena *
	arena_create_reserve(size_t reserve, size_t commit_step);

	// Destroys the arena and frees its memory
	void
	arena_destroy(Arena **arena);

	// Allocates memory from the arena
	void *
	arena_alloc(Arena *arena, size_t size);

	// Allocates memory from the arena with specified alignment
	void *
	arena_alloc_aligned(Arena *arena, size_t size, size_t alignment);

	// arena_alloc, zero-filled. Size 0 returns a valid (empty) pointer.
	void *
	arena_alloc_zero(Arena *arena, size_t size);

	// count * size bytes aligned to `alignment` (0 = default alignment), NULL on overflow.
	// Use the VA_PUSH_ARRAY / VA_PUSH_ARRAY_ZERO macros.
	void *
	arena_alloc_array(Arena *arena, size_t size, size_t count, size_t alignment);
	void *
	arena_alloc_array_zero(Arena *arena, size_t size, size_t count, size_t alignment);

	// Resizes an allocation: in place when `ptr` is the arena's most recent allocation and the
	// arena can hold the new size (always, for a reserved arena with address space left),
	// otherwise a new allocation with the first min(old_size, new_size) bytes copied. ptr NULL
	// allocates. Growing the last allocation in a loop (a string builder, a dynamic array) is
	// therefore amortised O(1) with no copying.
	void *
	arena_realloc(Arena *arena, void *ptr, size_t old_size, size_t new_size);

	// NUL-terminated copies and formatting into the arena.
	char *
	arena_strdup(Arena *arena, const char *s);
	char *
	arena_strndup(Arena *arena, const char *s, size_t n);
	char *
	arena_printf(Arena *arena, const char *fmt, ...) VA_PRINTF(2, 3);
	char *
	arena_vprintf(Arena *arena, const char *fmt, va_list ap);

	// Increases the arena size to accommodate more memory.
	// Malloc'd arenas: all pointers previously returned by arena_alloc/arena_alloc_aligned
	// may become invalid! Reserved arenas commit more pages in place (pointers stay valid), up to
	// the reservation. Returns a pointer to the current position in the new memory block, or NULL
	// on failure.
	void *
	arena_expand(Arena *arena, size_t new_size);

	// Resets the arena, freeing all allocated memory
	int
	arena_reset(Arena *arena);

	// Temp scopes: the position to return to, and freeing everything allocated since.
	size_t
	arena_pos(Arena *arena);
	void
	arena_pop_to(Arena *arena, size_t pos);
	ArenaTemp
	arena_temp_begin(Arena *arena);
	void
	arena_temp_end(ArenaTemp temp);

	// Returns the amount of memory currently used in the arena
	size_t
	arena_used_size(Arena *arena);

	// Returns the total size of the arena
	size_t
	arena_total_size(Arena *arena);

#ifdef __cplusplus
}
#endif

#endif // VA_ARENA_H

// ================== IMPLEMENTATION ==================
#ifdef VA_ARENA_IMPLEMENTATION
#ifndef VA_ARENA_IMPLEMENTED
#define VA_ARENA_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(VA_ARENA_NO_VM)
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
// strict -std=c99/c11 with another system header included first hides these; /dev/zero is the
// POSIX way to anonymous memory without them
#if !defined(MAP_NORESERVE)
#define MAP_NORESERVE 0
#endif
#endif
#endif

#if defined(__SANITIZE_ADDRESS__)
#define VA_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define VA_ASAN 1
#endif
#endif
#ifdef VA_ASAN
#include <sanitizer/asan_interface.h>
// out of line: GCC treats the const pointer parameter as a read of (fresh, uninitialized) memory
__attribute__((noinline)) static void
va_poison(void *p, size_t n)
{
	__asan_poison_memory_region(p, n);
}
#define VA_POISON(p, n)	  va_poison((p), (n))
#define VA_UNPOISON(p, n) __asan_unpoison_memory_region((p), (n))
#else
#define VA_POISON(p, n)	  ((void)(p), (void)(n))
#define VA_UNPOISON(p, n) ((void)(p), (void)(n))
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define VA_DEFAULT_ALIGNMENT VA_ALIGNOF(max_align_t)
#else
#define VA_DEFAULT_ALIGNMENT sizeof(void *)
#endif

#define VA_DEFAULT_COMMIT_STEP ((size_t)64 * 1024)
#define VA_NO_LAST			   SIZE_MAX
// Reserved arenas: the Arena header sits at the start of the reservation and memory follows it.
#define VA_HEADER_SIZE (((sizeof(Arena) + 63) / 64) * 64)

static size_t
align_up(size_t n, size_t align);

static int
va_add_overflow_size(size_t a, size_t b, size_t *out)
{
	if (a > SIZE_MAX - b)
	{
		return 1;
	}

	*out = a + b;
	return 0;
}

static int
va_is_power_of_two(size_t value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

// ---------- virtual memory (reserved arenas) ----------

static size_t
va_page_size(void)
{
#if defined(VA_ARENA_NO_VM)
	return 4096;
#elif defined(_WIN32)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return (size_t)si.dwPageSize;
#else
	long p = sysconf(_SC_PAGESIZE);
	return p > 0 ? (size_t)p : 4096;
#endif
}

static void *
va_vm_reserve(size_t n)
{
#if defined(VA_ARENA_NO_VM)
	(void)n;
	return NULL;
#elif defined(_WIN32)
	return VirtualAlloc(NULL, n, MEM_RESERVE, PAGE_NOACCESS);
#else
#if defined(MAP_ANONYMOUS)
	void *p = mmap(NULL, n, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
#else
	int fd = open("/dev/zero", O_RDWR);
	if (fd < 0)
	{
		return NULL;
	}
	void *p = mmap(NULL, n, PROT_NONE, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
	close(fd);
#endif
	return p == MAP_FAILED ? NULL : p;
#endif
}

// p and n are page-aligned. Nonzero on success.
static int
va_vm_commit(void *p, size_t n)
{
#if defined(VA_ARENA_NO_VM)
	(void)p;
	(void)n;
	return 0;
#elif defined(_WIN32)
	return VirtualAlloc(p, n, MEM_COMMIT, PAGE_READWRITE) != NULL;
#else
	return mprotect(p, n, PROT_READ | PROT_WRITE) == 0;
#endif
}

static void
va_vm_release(void *p, size_t n)
{
#if defined(VA_ARENA_NO_VM)
	(void)p;
	(void)n;
#elif defined(_WIN32)
	(void)n;
	VirtualFree(p, 0, MEM_RELEASE);
#else
	munmap(p, n);
#endif
}

// Makes [0, end) of the arena usable, committing pages of a reserved arena. 0 on success.
static int
va_ensure(Arena *arena, size_t end)
{
	if (end <= arena->size)
	{
		return 0;
	}
	if (!arena->reserved || end > arena->reserved)
	{
		return -1;
	}
	// commits are whole commit_steps measured from the (page-aligned) start of the reservation
	size_t target = align_up(VA_HEADER_SIZE + end, arena->commit_step);
	if (target == SIZE_MAX || target - VA_HEADER_SIZE > arena->reserved)
	{
		target = VA_HEADER_SIZE + arena->reserved;
	}
	target -= VA_HEADER_SIZE;
	if (!va_vm_commit(arena->memory + arena->size, target - arena->size))
	{
		return -1;
	}
	VA_POISON(arena->memory + arena->size, target - arena->size);
	arena->size = target;
	return 0;
}

Arena *
arena_create(size_t size)
{
	if (size == 0)
	{
		return NULL;
	}

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
	arena->reserved = 0;
	arena->commit_step = 0;
	arena->last = VA_NO_LAST;
	VA_POISON(arena->memory, size);

	return arena;
}

Arena *
arena_create_reserve(size_t reserve, size_t commit_step)
{
	size_t page = va_page_size();
	if (reserve == 0)
	{
		return NULL;
	}
	if (commit_step == 0)
	{
		commit_step = VA_DEFAULT_COMMIT_STEP;
	}
	if (commit_step > reserve)
	{
		commit_step = reserve;
	}
	commit_step = align_up(commit_step, page);
	size_t total = 0;
	if (commit_step == SIZE_MAX || va_add_overflow_size(reserve, VA_HEADER_SIZE, &total) || (total = align_up(total, page)) == SIZE_MAX)
	{
		return NULL;
	}

	uint8_t *base = (uint8_t *)va_vm_reserve(total);
	if (!base)
	{
		return NULL;
	}
	// the first commit holds the header plus the first commit_step of memory
	size_t first = align_up(VA_HEADER_SIZE + commit_step, page);
	if (first > total)
	{
		first = total;
	}
	if (!va_vm_commit(base, first))
	{
		va_vm_release(base, total);
		return NULL;
	}

	Arena *arena = (Arena *)base;
	arena->memory = base + VA_HEADER_SIZE;
	arena->reserved = total - VA_HEADER_SIZE;
	arena->size = first - VA_HEADER_SIZE;
	arena->used = 0;
	arena->commit_step = commit_step;
	arena->last = VA_NO_LAST;
	VA_POISON(arena->memory, arena->size);

	return arena;
}

void
arena_destroy(Arena **arena)
{
	if (arena && *arena)
	{
		// unpoisoned first: ASan's shadow outlives the mapping, and later mappings reuse addresses
		VA_UNPOISON((*arena)->memory, (*arena)->size);
		if ((*arena)->reserved)
		{
			va_vm_release(*arena, VA_HEADER_SIZE + (*arena)->reserved);
			*arena = NULL;
			return;
		}
		free((*arena)->memory);
		(*arena)->memory = NULL;
		(*arena)->used = 0;
		free(*arena);
		*arena = NULL;
	}
}

void *
arena_alloc(Arena *arena, size_t size)
{
	if (!arena || size == 0)
	{
		return NULL;
	}

	size_t aligned_used = align_up(arena->used, VA_DEFAULT_ALIGNMENT);
	size_t end_offset = 0;

	if (va_add_overflow_size(aligned_used, size, &end_offset) || va_ensure(arena, end_offset) != 0)
	{
		return NULL;
	}

	void *ptr = arena->memory + aligned_used;
	arena->used = end_offset;
	arena->last = aligned_used;
	VA_UNPOISON(ptr, size);

	return ptr;
}

static size_t
align_up(size_t n, size_t align)
{
	// Assumes alignment is a power of two.
	// The & ~(align - 1) operation rounds n up to the nearest multiple of align.
	size_t addend = align - 1;
	size_t rounded = 0;

	if (va_add_overflow_size(n, addend, &rounded))
	{
		return SIZE_MAX;
	}

	return rounded & ~(align - 1);
}

void *
arena_alloc_aligned(Arena *arena, size_t size, size_t alignment)
{
	if (!arena || size == 0 || !va_is_power_of_two(alignment))
	{
		return NULL;
	}

	uintptr_t base = (uintptr_t)arena->memory;
	size_t offset = arena->used;

	// Pad offset so that (base + offset) is aligned to 'alignment'.
	// align_up alone is insufficient when arena->memory (from malloc)
	// is not itself aligned to the requested alignment.
	size_t misalign = (size_t)((base + offset) & (alignment - 1));
	if (misalign != 0)
	{
		size_t pad = alignment - misalign;
		if (va_add_overflow_size(offset, pad, &offset))
		{
			return NULL;
		}
	}

	size_t end_offset = 0;
	if (va_add_overflow_size(offset, size, &end_offset) || va_ensure(arena, end_offset) != 0)
	{
		return NULL;
	}

	void *ptr = arena->memory + offset;
	arena->used = end_offset;
	arena->last = offset;
	VA_UNPOISON(ptr, size);

	return ptr;
}

// Size 0 gets a valid empty allocation (1 byte reserved, so the pointer is unique and in bounds).
static void *
va_alloc_any(Arena *arena, size_t size, size_t alignment)
{
	if (alignment == 0)
	{
		alignment = VA_DEFAULT_ALIGNMENT;
	}
	return arena_alloc_aligned(arena, size ? size : 1, alignment);
}

void *
arena_alloc_zero(Arena *arena, size_t size)
{
	void *p = va_alloc_any(arena, size, 0);
	if (p)
	{
		memset(p, 0, size);
	}
	return p;
}

void *
arena_alloc_array(Arena *arena, size_t size, size_t count, size_t alignment)
{
	if (size && count > SIZE_MAX / size)
	{
		return NULL;
	}
	return va_alloc_any(arena, size * count, alignment);
}

void *
arena_alloc_array_zero(Arena *arena, size_t size, size_t count, size_t alignment)
{
	void *p = arena_alloc_array(arena, size, count, alignment);
	if (p)
	{
		memset(p, 0, size * count);
	}
	return p;
}

void *
arena_realloc(Arena *arena, void *ptr, size_t old_size, size_t new_size)
{
	if (!arena)
	{
		return NULL;
	}
	if (!ptr)
	{
		return va_alloc_any(arena, new_size, 0);
	}
	size_t offset = (size_t)((uint8_t *)ptr - arena->memory);
	if (arena->last == offset)
	{
		size_t end = 0;
		size_t need = new_size ? new_size : 1;
		if (!va_add_overflow_size(offset, need, &end) && va_ensure(arena, end) == 0)
		{
			if (end < arena->used)
			{
				VA_POISON(arena->memory + end, arena->used - end);
			}
			arena->used = end;
			VA_UNPOISON(ptr, need);
			return ptr;
		}
	}
	void *p = va_alloc_any(arena, new_size, 0);
	if (p)
	{
		memcpy(p, ptr, old_size < new_size ? old_size : new_size);
	}
	return p;
}

char *
arena_strndup(Arena *arena, const char *s, size_t n)
{
	size_t len = 0;
	while (len < n && s[len])
	{
		len++;
	}
	char *p = (char *)arena_alloc_aligned(arena, len + 1, 1);
	if (p)
	{
		memcpy(p, s, len);
		p[len] = 0;
	}
	return p;
}

char *
arena_strdup(Arena *arena, const char *s)
{
	return arena_strndup(arena, s, SIZE_MAX);
}

char *
arena_vprintf(Arena *arena, const char *fmt, va_list ap)
{
	va_list copy;
	va_copy(copy, ap);
	int n = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);
	if (n < 0)
	{
		return NULL;
	}
	char *p = (char *)arena_alloc_aligned(arena, (size_t)n + 1, 1);
	if (p)
	{
		vsnprintf(p, (size_t)n + 1, fmt, ap);
	}
	return p;
}

char *
arena_printf(Arena *arena, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char *p = arena_vprintf(arena, fmt, ap);
	va_end(ap);
	return p;
}

void *
arena_expand(Arena *arena, size_t new_size)
{
	if (!arena || new_size <= arena->size)
	{
		return NULL;
	}

	if (arena->reserved)
	{
		if (va_ensure(arena, new_size) != 0)
		{
			return NULL;
		}
		return arena->memory + arena->used;
	}

	VA_UNPOISON(arena->memory, arena->size);
	uint8_t *new_memory = (uint8_t *)realloc(arena->memory, new_size);
	if (!new_memory)
	{
		VA_POISON(arena->memory + arena->used, arena->size - arena->used);
		return NULL; // Reallocation failed
	}

	arena->memory = new_memory;
	arena->size = new_size;
	VA_POISON(arena->memory + arena->used, arena->size - arena->used);

	return arena->memory + arena->used;
}

int
arena_reset(Arena *arena)
{
	if (!arena || !arena->memory)
	{
		return -1;
	}

	arena->used = 0;
	arena->last = VA_NO_LAST;
	VA_POISON(arena->memory, arena->size);

	return 0;
}

size_t
arena_pos(Arena *arena)
{
	return arena ? arena->used : 0;
}

void
arena_pop_to(Arena *arena, size_t pos)
{
	if (!arena || pos >= arena->used)
	{
		return;
	}
	VA_POISON(arena->memory + pos, arena->used - pos);
	arena->used = pos;
	if (arena->last != VA_NO_LAST && arena->last >= pos)
	{
		arena->last = VA_NO_LAST;
	}
}

ArenaTemp
arena_temp_begin(Arena *arena)
{
	ArenaTemp t = {arena, arena_pos(arena), arena ? arena->last : VA_NO_LAST};
	return t;
}

void
arena_temp_end(ArenaTemp temp)
{
	arena_pop_to(temp.arena, temp.pos);
	if (temp.arena)
	{
		temp.arena->last = temp.last;
	}
}

size_t
arena_used_size(Arena *arena)
{
	if (!arena)
	{
		return 0;
	}

	return arena->used;
}

size_t
arena_total_size(Arena *arena)
{
	if (!arena)
	{
		return 0;
	}

	return arena->size;
}

#endif // VA_ARENA_IMPLEMENTED
#endif // VA_ARENA_IMPLEMENTATION
