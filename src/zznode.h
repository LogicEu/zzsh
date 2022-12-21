#ifndef ZZ_NODE_H
#define ZZ_NODE_H

#include <zztypes.h>

struct zznode {
    char* data;
    size_t len;
};

struct zznode zznode_create(const char* name, const void* data, const size_t size);
struct zznode zznode_copy(const struct zznode* node);
void zznode_push(struct zznode* node, const void* data, const size_t size);
void zznode_remove(struct zznode* node, size_t from, size_t to);
void zznode_free(struct zznode* node);
void zznode_clear(struct zznode* node);
void* zznode_data(const struct zznode* node);
void zznode_rename(struct zznode* node, const char* name);
const char* zznode_name(const struct zznode* node);
int zznode_isdir(const struct zznode* node);

#endif /* ZZ_NODE_H */
