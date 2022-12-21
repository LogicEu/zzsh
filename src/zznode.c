#include <zznode.h>
#include <stdlib.h>
#include <string.h>

static size_t zzallign(size_t n)
{
    while (n % sizeof(void*)) { ++n; }
    return n;
}

struct zznode zznode_create(const char* name, const void* data, const size_t size)
{
    struct zznode node;
    
    const size_t len = strlen(name);
    const size_t allign = zzallign(len + 1);
    
    node.data = calloc(size + allign, 1);
    node.len = size;

    memcpy(node.data, name, len);
    if (size && data) {
        memcpy(node.data + allign, data, size);
    }

    return node;
}

struct zznode zznode_copy(const struct zznode* node)
{
    return zznode_create(node->data, zznode_data(node), node->len);
}

void zznode_push(struct zznode* node, const void* data, const size_t size)
{
    const size_t len = strlen(node->data);
    const size_t allign = zzallign(len + 1);

    node->data = realloc(node->data, node->len + allign + size);
    memcpy(node->data + allign + node->len, data, size);
    node->len += size;
}

void zznode_remove(struct zznode* node, size_t from, size_t to)
{
    const size_t dif = to - from;
    if (dif <= node->len) {
        memmove(node->data + to, node->data + from, node->len - to);
        node->len -= dif;
        node->data = realloc(node->data, node->len);
    }
}

void zznode_free(struct zznode* node)
{
    free(node->data);
    node->data = NULL;
    node->len = 0;
}

void zznode_clear(struct zznode* node)
{
    const size_t len = strlen(node->data) + 1;
    node->data = realloc(node->data, len);
}

void* zznode_data(const struct zznode* node)
{
    return node->len ? node->data + zzallign(strlen(node->data) + 1) : NULL;
}

void zznode_rename(struct zznode* node, const char* name)
{
    const size_t len = strlen(node->data);
    const size_t allign = zzallign(len + 1);
    const size_t size = strlen(name);
    const size_t new_allign = zzallign(size + 1);

    if (allign != new_allign) {
        memmove(node->data + new_allign, node->data + allign, node->len);
        node->data = realloc(node->data, node->len + new_allign);
    }

    memcpy(node->data, name, size);
    memset(node->data + size, 0, new_allign - size);
}

const char* zznode_name(const struct zznode* node)
{
    return node->data;
}
