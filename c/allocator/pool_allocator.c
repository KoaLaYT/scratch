/*
 * A fixed-sized pool allocator.
 *
 * https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
 */

#include <string.h>

#include "helper.h"

typedef struct node_t node_t;
struct node_t {
    node_t* next;
};

typedef struct {
    unsigned char* buf;
    size_t buf_len;
    size_t chunk_size;
    node_t* head;
} pool_t;

// api
pool_t pool_init(void* backing_buffer, size_t buf_len,  //
                 size_t chunk_size, size_t align);
void* pool_alloc(pool_t* p);
void pool_free(pool_t* p, void* ptr);
void pool_free_all(pool_t* p);

// impl
pool_t pool_init(void* backing_buffer, size_t buf_len,  //
                 size_t chunk_size, size_t align)
{
    uintptr_t init_start = (uintptr_t)backing_buffer;
    uintptr_t start = align_forward(init_start, align);
    buf_len -= (size_t)(start - init_start);

    chunk_size = MAX(sizeof(node_t), chunk_size);
    chunk_size = align_forward(chunk_size, align);

    assert(buf_len >= chunk_size);

    pool_t p = {0};
    p.buf = (unsigned char*)start;
    p.buf_len = buf_len;
    p.chunk_size = chunk_size;

    pool_free_all(&p);
    return p;
}

void* pool_alloc(pool_t* p)
{
    void* ptr = (void*)p->head;

    // oom
    if (ptr == 0) {
        return 0;
    }

    p->head = p->head->next;

    return memset(ptr, 0, p->chunk_size);
}

void pool_free(pool_t* p, void* ptr)
{
    if (ptr == 0) return;

    assert(p->buf <= (unsigned char*)ptr &&
           (unsigned char*)ptr < p->buf + p->buf_len);

    node_t* node = (node_t*)ptr;
    node->next = p->head;
    p->head = node;
}

void pool_free_all(pool_t* p)
{
    p->head = 0;
    size_t count = p->buf_len / p->chunk_size;
    for (size_t i = 0; i < count; i++) {
        node_t* node = (node_t*)&p->buf[i * p->chunk_size];
        node->next = p->head;
        p->head = node;
    }
}

// demo
int main(int argc, char** argv)
{
    int i;
    unsigned char backing_buffer[1024];
    void *a, *b, *c, *d, *e, *f;

    pool_t p = pool_init(backing_buffer, 1024, 64, 16);

    a = pool_alloc(&p);
    b = pool_alloc(&p);
    c = pool_alloc(&p);
    d = pool_alloc(&p);
    e = pool_alloc(&p);
    f = pool_alloc(&p);

    pool_free(&p, f);
    pool_free(&p, c);
    pool_free(&p, b);
    pool_free(&p, d);

    d = pool_alloc(&p);

    pool_free(&p, a);

    a = pool_alloc(&p);

    pool_free(&p, e);
    pool_free(&p, a);
    pool_free(&p, d);

    return 0;
}
