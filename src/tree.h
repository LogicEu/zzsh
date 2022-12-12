#ifndef TREE_H
#define TREE_H

#include <stddef.h>

struct tnode_t {
    struct tnode_t** children;
    struct tnode_t* parent;
    void* data;
};

struct tree_t {
    struct tnode_t* root;
    size_t bytes;
};

size_t tnode_children_count(const struct tnode_t* node);
size_t tnode_children_capacity(const size_t children_count);
struct tnode_t* tnode_create(const void* data, const size_t bytes);
void tnode_free(struct tnode_t* node);
void tnode_push(struct tnode_t* root, struct tnode_t* leave);
int tnode_remove(struct tnode_t* root, const size_t index);

struct tree_t tree_create(const size_t bytes);
void tree_free(struct tree_t* tree);
void tree_push_root(struct tree_t* tree, const void* data);
void tree_push_leave(struct tree_t* tree, const void* data);
struct tnode_t* tnode_root(struct tnode_t* node);

#endif /* TREE_H */
