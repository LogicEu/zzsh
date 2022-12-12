#ifndef MAP_H
#define MAP_H

#include <stddef.h>

#ifndef MAP_SIZE
    #define MAP_SIZE 32
#endif

#define BUCKET_CAP_INDEX 0
#define BUCKET_SIZE_INDEX 1
#define BUCKET_DATA_INDEX 2

typedef size_t index_t;
typedef index_t* bucket_t;

struct map_t {
    bucket_t* indices;
    void* keys;
    void* values;
    size_t key_bytes;
    size_t value_bytes;
    size_t size;
    size_t mod;
    size_t (*func)(const void*);
};

#define _map_key_at(map, i) ((char*)(map)->keys + (map)->key_bytes * (i))
#define _map_value_at(map, i) ((char*)(map)->values + (map)->value_bytes * (i))

struct map_t map_create(const size_t key_size, const size_t value_size);
struct map_t map_reserve(const size_t key_size, const size_t value_size, const size_t reserve);
struct map_t map_copy(const struct map_t* map);
index_t map_search(const struct map_t* map, const void* key);
index_t* map_search_all(const struct map_t* map, const void* key);
void map_overload(struct map_t* map, size_t (*hash_func)(const void*));
void* map_key_at(const struct map_t* map, const size_t index);
void* map_value_at(const struct map_t* map, const size_t index);
size_t map_size(const struct map_t* map);
size_t map_capacity(const struct map_t* map);
size_t map_key_bytes(const struct map_t* map);
size_t map_value_bytes(const struct map_t* map);
void map_resize(struct map_t* map, const size_t size);
void map_push(struct map_t* map, const void* key, const void* value);
size_t map_push_if(struct map_t* map, const void* key, const void* value);
void map_remove(struct map_t* map, const void* key);
void map_free(struct map_t* map);

size_t map_hash_cstr(const void* key);
size_t map_hash_uint(const void* key);

#endif /* MAP_H */
