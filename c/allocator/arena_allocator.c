/*
 * A linear/arena memory allocator.
 *
 * https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
 */

#include <stdalign.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"

typedef struct {
    unsigned char* buf;
    size_t buf_len;
    size_t prev_offset;
    size_t curr_offset;
} arena_t;

// api
arena_t arena_init(void* backing_buffer, size_t buf_len);
void* arena_alloc_align(arena_t* a, size_t size, size_t align);
void* arena_resize_align(arena_t* a, void* old_memory, size_t old_size,  //
                         size_t new_size, size_t align);
void arena_reset(arena_t* a);

#define arena_alloc(a, t, n) \
    (t*)arena_alloc_align(a, sizeof(t) * (n), alignof(t))
#define arena_create(a, t) arena_alloc(a, t, 1)
#define arena_resize(a, old_mem, old_num, t, new_num)         \
    (t*)arena_resize_align(a, old_mem, sizeof(t) * (old_num), \
                           sizeof(t) * (new_num), alignof(t));

// impl
arena_t arena_init(void* backing_buffer, size_t buf_len)
{
    arena_t a = {0};
    a.buf = backing_buffer;
    a.buf_len = buf_len;
    return a;
}

void* arena_alloc_align(arena_t* a, size_t size, size_t align)
{
    uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
    uintptr_t offset = align_forward(curr_ptr, align);
    offset -= (uintptr_t)a->buf;

    if (offset + size <= a->buf_len) {
        void* ptr = &a->buf[offset];
        a->prev_offset = offset;
        a->curr_offset = offset + size;
        memset(ptr, 0, size);
        return ptr;
    }

    // one possible oom policy
    return 0;
}

void* arena_resize_align(arena_t* a, void* old_memory, size_t old_size,  //
                         size_t new_size, size_t align)
{
    assert(is_power_of_two(align));

    unsigned char* old_mem = (unsigned char*)old_memory;

    if (old_memory == 0) {
        return arena_alloc_align(a, new_size, align);
    }

    assert(a->buf <= old_mem && old_mem < a->buf + a->buf_len);

    if (a->buf + a->prev_offset == old_mem) {
        // oom
        if (a->prev_offset + new_size > a->buf_len) {
            return 0;
        }
        if (new_size > old_size) {
            memset(&a->buf[a->curr_offset], 0, new_size - old_size);
        }
        a->curr_offset = a->prev_offset + new_size;
        return old_memory;
    } else {
        void* new_memory = arena_alloc_align(a, new_size, align);
        size_t copy_size = old_size < new_size ? old_size : new_size;
        memmove(new_memory, old_memory, copy_size);
        return new_memory;
    }
}

void arena_reset(arena_t* a)
{
    a->prev_offset = 0;
    a->curr_offset = 0;
}

// demo
int main(int argc, char** argv)
{
    unsigned char backing_buffer[256];
    arena_t a = arena_init(backing_buffer, 256);

    for (size_t i = 0; i < 10; i++) {
        // Reset all arena offsets for each loop
        arena_reset(&a);

        int* x = arena_create(&a, int);
        float* f = arena_create(&a, float);
        char* str = arena_alloc(&a, char, 10);

        *x = 123;
        *f = 987;
        memmove(str, "Hellope", 7);

        printf("%p: %d\n", x, *x);
        printf("%p: %f\n", f, *f);
        printf("%p: %s\n", str, str);

        str = arena_resize(&a, str, 10, char, 32);
        memmove(str + 7, " world!", 7);
        printf("%p: %s\n", str, str);
    }

    return 0;
}
