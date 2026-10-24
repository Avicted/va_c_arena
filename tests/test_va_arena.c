#define VA_ARENA_IMPLEMENTATION
#include "va_arena.h"

#include <stdalign.h>
#include <stdio.h>
#include <string.h>

int
test_arena_create(void)
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

int
test_arena_alloc(void)
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

int
test_arena_alloc_exceed(void)
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

int
test_arena_alloc_aligned(void)
{
	Arena *arena = arena_create(1024);
	void *ptr = arena_alloc_aligned(arena, 64, VA_ALIGNOF(long double));
	if (ptr == NULL)
	{
		printf("test_arena_alloc_aligned\t\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}
	else if (((uintptr_t)ptr) % VA_ALIGNOF(long double) != 0)
	{
		printf("test_arena_alloc_aligned\t\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}
	else
	{
		printf("test_arena_alloc_aligned\t\tpassed\n");
		arena_destroy(&arena);
		return 0;
	}
}

int
test_arena_alloc_aligned_large(void)
{
	// Verify alignment larger than alignof(max_align_t),
	// which exposes bugs where base-address misalignment is ignored.
	Arena *arena = arena_create(2048);
	void *ptr = arena_alloc_aligned(arena, 1024, 32);
	if (ptr == NULL)
	{
		printf("test_arena_alloc_aligned_large\t\tfailed (NULL ptr)\n");
		arena_destroy(&arena);
		return -1;
	}
	else if (((uintptr_t)ptr) % 32 != 0)
	{
		printf("test_arena_alloc_aligned_large\t\tfailed (misaligned)\n");
		arena_destroy(&arena);
		return -1;
	}
	else
	{
		printf("test_arena_alloc_aligned_large\t\tpassed\n");
		arena_destroy(&arena);
		return 0;
	}
}

int
test_arena_alloc_aligned_exceed(void)
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

int
test_arena_destroy(void)
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

int
test_arena_reset(void)
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

int
test_arena_used_and_total_size(void)
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

int
test_arena_alignment(void)
{
	Arena *arena = arena_create(1024);
	if (!arena)
	{
		printf("test_arena_alignment\t\t\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}

	void *ptr1 = arena_alloc(arena, 1);
	if (!ptr1)
	{
		printf("test_arena_alignment\t\t\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}

	void *ptr2 = arena_alloc(arena, sizeof(double));
	if (!ptr2)
	{
		printf("test_arena_alignment\t\t\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}

	if (((uintptr_t)ptr2) % VA_ALIGNOF(double) != 0)
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

int
test_arena_alloc_aligned_invalid_alignment(void)
{
	Arena *arena = arena_create(1024);
	if (!arena)
	{
		printf("test_arena_aligned_invalid_align\tfailed\n");
		return -1;
	}

	if (arena_alloc_aligned(arena, 64, 3) != NULL)
	{
		printf("test_arena_aligned_invalid_align\tfailed\n");
		arena_destroy(&arena);
		return -1;
	}

	printf("test_arena_aligned_invalid_align\tpassed\n");
	arena_destroy(&arena);
	return 0;
}

int
test_arena_expand(void)
{
	Arena *arena = arena_create(128);
	if (!arena)
	{
		printf("test_arena_expand_happy\t\t\tfailed (arena_create)\n");
		return -1;
	}

	void *ptr1 = arena_alloc(arena, 64);
	if (!ptr1)
	{
		printf("test_arena_expand_happy\t\t\tfailed (arena_alloc)\n");
		arena_destroy(&arena);
		return -1;
	}

	void *expand_ptr = arena_expand(arena, 256);
	if (!expand_ptr || arena_total_size(arena) != 256)
	{
		printf("test_arena_expand_happy\t\t\tfailed (arena_expand)\n");
		arena_destroy(&arena);
		return -1;
	}

	// Should still be able to allocate more after expand
	void *ptr2 = arena_alloc(arena, 128);
	if (!ptr2)
	{
		printf("test_arena_expand_happy\t\t\tfailed (post-expand alloc)\n");
		arena_destroy(&arena);
		return -1;
	}

	printf("test_arena_expand_happy\t\t\tpassed\n");
	arena_destroy(&arena);
	return 0;
}

int
test_arena_expand_fail(void)
{
	Arena *arena = arena_create(128);
	if (!arena)
	{
		printf("test_arena_expand_fail\t\t\tfailed (arena_create)\n");
		return -1;
	}

	// Try to shrink (should fail)
	void *expand_ptr = arena_expand(arena, 64);
	if (expand_ptr != NULL)
	{
		printf("test_arena_expand_fail\t\t\tfailed (shrink allowed)\n");
		arena_destroy(&arena);
		return -1;
	}

	// Try to expand with NULL arena (should fail)
	if (arena_expand(NULL, 256) != NULL)
	{
		printf("test_arena_expand_fail\t\t\tfailed (NULL arena allowed)\n");
		arena_destroy(&arena);
		return -1;
	}

	printf("test_arena_expand_fail\t\t\tpassed\n");
	arena_destroy(&arena);
	return 0;
}

// ---------- 0.4: reserved (virtual memory) arenas, temp scopes, realloc, strings ----------

static int
report(const char *name, const char *failure)
{
	if (failure)
	{
		printf("%-40s failed (%s)\n", name, failure);
		return -1;
	}
	printf("%-40s passed\n", name);
	return 0;
}

#define MB(x) ((size_t)(x) * 1024 * 1024)

int
test_reserve_create(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(1024), 0);
	if (!arena)
	{
		return report("test_reserve_create", "arena_create_reserve");
	}
	if (arena->reserved < MB(1024))
	{
		fail = "reserved less than asked";
	}
	else if (arena_total_size(arena) == 0 || arena_total_size(arena) > MB(1))
	{
		fail = "first commit should be small";
	}
	else if (((uintptr_t)arena->memory & 63) != 0)
	{
		fail = "memory not 64-byte aligned";
	}
	arena_destroy(&arena);
	if (!fail && arena != NULL)
	{
		fail = "destroy did not clear the pointer";
	}
	if (!fail && arena_create_reserve(0, 0) != NULL)
	{
		fail = "zero reserve accepted";
	}
	return report("test_reserve_create", fail);
}

int
test_reserve_grows_in_place(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(256), 64 * 1024);
	uint8_t *memory = arena->memory;
	uint32_t *first = (uint32_t *)arena_alloc(arena, 4096);
	for (uint32_t i = 0; i < 1024; i++)
	{
		first[i] = i * 2654435761u;
	}
	// 32 MB more, one page at a time: commits on demand, nothing moves
	for (int i = 0; i < 8192 && !fail; i++)
	{
		uint8_t *p = (uint8_t *)arena_alloc(arena, 4096);
		if (!p)
		{
			fail = "alloc failed while committing";
		}
		else
		{
			memset(p, 0xab, 4096);
		}
	}
	if (!fail && arena->memory != memory)
	{
		fail = "memory moved";
	}
	for (uint32_t i = 0; i < 1024 && !fail; i++)
	{
		if (first[i] != i * 2654435761u)
		{
			fail = "earlier allocation corrupted";
		}
	}
	if (!fail && arena_total_size(arena) < arena_used_size(arena))
	{
		fail = "used beyond committed";
	}
	arena_destroy(&arena);
	return report("test_reserve_grows_in_place", fail);
}

int
test_reserve_exhausted(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(1), 0);
	if (arena_alloc(arena, MB(2)) != NULL)
	{
		fail = "allocated beyond the reservation";
	}
	else if (!arena_alloc(arena, MB(1) / 2))
	{
		fail = "arena unusable after a failed allocation";
	}
	else if (arena_alloc(arena, MB(1)) != NULL)
	{
		fail = "second allocation beyond the reservation";
	}
	arena_destroy(&arena);
	return report("test_reserve_exhausted", fail);
}

int
test_reserve_expand(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	uint8_t *memory = arena->memory;
	char *s = arena_strdup(arena, "kept");
	if (!arena_expand(arena, MB(8)))
	{
		fail = "expand failed";
	}
	else if (arena->memory != memory || strcmp(s, "kept"))
	{
		fail = "expand moved memory";
	}
	else if (arena_total_size(arena) < MB(8))
	{
		fail = "expand committed too little";
	}
	else if (arena_expand(arena, MB(65)) != NULL)
	{
		fail = "expanded beyond the reservation";
	}
	arena_destroy(&arena);
	return report("test_reserve_expand", fail);
}

int
test_temp_scope(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	char *keep = arena_strdup(arena, "keep");
	size_t before = arena_pos(arena);
	ArenaTemp outer = arena_temp_begin(arena);
	arena_alloc(arena, 1000);
	ArenaTemp inner = arena_temp_begin(arena);
	arena_alloc(arena, MB(4));
	arena_temp_end(inner);
	if (arena_pos(arena) != inner.pos)
	{
		fail = "inner scope not popped";
	}
	arena_temp_end(outer);
	if (!fail && arena_pos(arena) != before)
	{
		fail = "outer scope not popped";
	}
	if (!fail && strcmp(keep, "keep"))
	{
		fail = "allocation before the scope damaged";
	}
	// the allocation before the scope is the last one again: it can still grow in place
	if (!fail && arena_realloc(arena, keep, 5, 100) != keep)
	{
		fail = "last allocation not restored by temp_end";
	}
	arena_pop_to(arena, 0);
	if (!fail && arena_used_size(arena) != 0)
	{
		fail = "pop_to 0";
	}
	arena_destroy(&arena);
	return report("test_temp_scope", fail);
}

int
test_alloc_zero_and_arrays(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	ArenaTemp t = arena_temp_begin(arena);
	memset(arena_alloc(arena, 4096), 0xff, 4096);
	arena_temp_end(t);
	uint8_t *z = (uint8_t *)arena_alloc_zero(arena, 4096);
	for (int i = 0; i < 4096 && !fail; i++)
	{
		if (z[i])
		{
			fail = "arena_alloc_zero not zeroed";
		}
	}
	if (!fail && (!arena_alloc_zero(arena, 0) || !VA_PUSH_ARRAY(arena, int, 0)))
	{
		fail = "size 0 must return a valid pointer";
	}
	double *d = VA_PUSH_ARRAY(arena, double, 100);
	if (!fail && (!d || ((uintptr_t)d % VA_ALIGNOF(double))))
	{
		fail = "VA_PUSH_ARRAY alignment";
	}
	long double *ld = VA_PUSH_ARRAY_ZERO(arena, long double, 3);
	if (!fail && (!ld || ((uintptr_t)ld % VA_ALIGNOF(long double)) || ld[2] != 0))
	{
		fail = "VA_PUSH_ARRAY_ZERO";
	}
	if (!fail && VA_PUSH_ARRAY(arena, uint64_t, SIZE_MAX / 4) != NULL)
	{
		fail = "count * size overflow not caught";
	}
	struct point
	{
		int x, y;
	};
	struct point *pt = VA_PUSH_STRUCT_ZERO(arena, struct point);
	if (!fail && (!pt || pt->x || pt->y))
	{
		fail = "VA_PUSH_STRUCT_ZERO";
	}
	arena_destroy(&arena);
	return report("test_alloc_zero_and_arrays", fail);
}

int
test_realloc(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	char *a = (char *)arena_alloc(arena, 16);
	strcpy(a, "hello");
	if (arena_realloc(arena, a, 16, MB(2)) != a)
	{
		fail = "last allocation did not grow in place";
	}
	if (!fail && arena_realloc(arena, a, MB(2), 8) != a)
	{
		fail = "shrink not in place";
	}
	if (!fail && arena_used_size(arena) != (size_t)(a - (char *)arena->memory) + 8)
	{
		fail = "shrink did not release the tail";
	}
	char *b = arena_strdup(arena, "other");
	char *a2 = (char *)arena_realloc(arena, a, 8, 64);
	if (!fail && (a2 == a || strcmp(a2, "hello")))
	{
		fail = "non-last allocation must move with its contents";
	}
	if (!fail && strcmp(b, "other"))
	{
		fail = "moving damaged the next allocation";
	}
	char *n = (char *)arena_realloc(arena, NULL, 0, 10);
	if (!fail && !n)
	{
		fail = "realloc(NULL) must allocate";
	}
	// a string builder growing one byte at a time never copies
	char *sb = (char *)arena_realloc(arena, NULL, 0, 1);
	for (size_t len = 1; len < 100000 && !fail; len++)
	{
		if (arena_realloc(arena, sb, len, len + 1) != sb)
		{
			fail = "builder moved";
		}
		sb[len - 1] = 'x';
	}
	arena_destroy(&arena);

	// malloc'd arenas grow in place too, while the block has room
	Arena *small = arena_create(1024);
	char *s = (char *)arena_alloc(small, 10);
	if (!fail && arena_realloc(small, s, 10, 500) != s)
	{
		fail = "malloc'd arena: no in-place growth";
	}
	if (!fail && arena_realloc(small, s, 500, 2048) != NULL)
	{
		fail = "malloc'd arena: grew beyond its block";
	}
	arena_destroy(&small);
	return report("test_realloc", fail);
}

int
test_strings(void)
{
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	char *a = arena_strdup(arena, "pre-commit");
	char *b = arena_strndup(arena, "pre-commit", 3);
	char *c = arena_strndup(arena, "ab", 10);
	char *d = arena_printf(arena, "%s=%d (%zu)", "hooks", 42, (size_t)7);
	char *e = arena_printf(arena, "%s", "");
	char big[5000];
	memset(big, 'y', sizeof big - 1);
	big[sizeof big - 1] = 0;
	char *f = arena_printf(arena, "<%s>", big);
	if (strcmp(a, "pre-commit") || strcmp(b, "pre") || strcmp(c, "ab"))
	{
		fail = "strdup/strndup";
	}
	else if (strcmp(d, "hooks=42 (7)") || !e || e[0])
	{
		fail = "printf";
	}
	else if (strlen(f) != sizeof big + 1 || f[0] != '<' || f[sizeof big] != '>')
	{
		fail = "printf longer than the commit step";
	}
	arena_destroy(&arena);
	return report("test_strings", fail);
}

int
test_asan_poisoning(void)
{
#ifdef VA_ASAN
	const char *fail = NULL;
	Arena *arena = arena_create_reserve(MB(64), 0);
	char *p = (char *)arena_alloc(arena, 32);
	if (__asan_address_is_poisoned(p) || __asan_address_is_poisoned(p + 31))
	{
		fail = "allocation poisoned";
	}
	else if (!__asan_address_is_poisoned(arena->memory + arena->used + 64))
	{
		fail = "free tail not poisoned";
	}
	ArenaTemp t = arena_temp_begin(arena);
	char *q = (char *)arena_alloc(arena, 64);
	arena_temp_end(t);
	if (!fail && !__asan_address_is_poisoned(q + 16))
	{
		fail = "popped memory not poisoned";
	}
	arena_destroy(&arena);
	return report("test_asan_poisoning", fail);
#else
	return report("test_asan_poisoning (not an ASan build)", NULL);
#endif
}

int
main(void)
{
	printf("va_c_arena - Arena Memory Management Tests\n\n");

	size_t success_count = 0;
	size_t error_count = 0;

	int (*tests[])(void) = {test_arena_create,
							test_arena_alloc,
							test_arena_alloc_exceed,
							test_arena_alloc_aligned,
							test_arena_alloc_aligned_large,
							test_arena_alloc_aligned_exceed,
							test_arena_alloc_aligned_invalid_alignment,
							test_arena_destroy,
							test_arena_reset,
							test_arena_used_and_total_size,
							test_arena_alignment,
							test_arena_expand,
							test_arena_expand_fail,
							test_reserve_create,
							test_reserve_grows_in_place,
							test_reserve_exhausted,
							test_reserve_expand,
							test_temp_scope,
							test_alloc_zero_and_arrays,
							test_realloc,
							test_strings,
							test_asan_poisoning};

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

	return error_count ? 1 : 0;
}
