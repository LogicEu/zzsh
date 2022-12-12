#include <map.h>
#include <stdlib.h>
#include <string.h>

size_t map_hash_cstr(const void* key)
{
    unsigned char* str = *(unsigned char**)(size_t)key;
    
    size_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

size_t map_hash_uint(const void* key) 
{
    size_t x = *(size_t*)key;
    x = ((x >> 16) ^ x) * 0x45d9f3b; 
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    return (x >> 16) ^ x;
}

static size_t bucket_size(const bucket_t bucket)
{
    return bucket ? bucket[BUCKET_SIZE_INDEX] : 0;
}

static size_t bucket_capacity(const bucket_t bucket)
{
    return bucket ? bucket[BUCKET_CAP_INDEX] : 0;
}

static void bucket_reindex(bucket_t bucket, const index_t removed)
{
    size_t i;
    const size_t count = bucket_size(bucket) + BUCKET_DATA_INDEX;
    for (i = BUCKET_DATA_INDEX; i < count; i++) {
        bucket[i] -= (bucket[i] >= removed);
    }
}

static bucket_t bucket_push(bucket_t bucket, const index_t index)
{
    const size_t size = bucket_size(bucket);
    const size_t cap = bucket_capacity(bucket);

    if (size == cap) {
        const size_t capacity = cap * 2 + !cap;
        bucket = realloc(bucket, (capacity + BUCKET_DATA_INDEX) * sizeof(index_t));
        bucket[BUCKET_CAP_INDEX] = capacity;
    }

    bucket[BUCKET_DATA_INDEX + size] = index;
    bucket[BUCKET_SIZE_INDEX] = size + 1;

    return bucket;
}

static void bucket_remove(bucket_t bucket, const size_t index)
{
    const size_t size = bucket_size(bucket) + BUCKET_DATA_INDEX;
    index_t* ptr = bucket + index;
    memmove(ptr, ptr + 1, (size - index - 1) * sizeof(index_t));
    bucket[BUCKET_SIZE_INDEX] = size - 1 - BUCKET_DATA_INDEX;
}

static void buckets_reindex(bucket_t*buckets, const size_t size, const index_t removed)
{
    size_t i;
    for (i = 0; i < size; ++i) {
        if (buckets[i]) {
            bucket_reindex(buckets[i], removed);
        }
    }
}

static void buckets_free(bucket_t*buckets, const size_t size)
{
    size_t i;
    for (i = 0; i < size; ++i) {
        if (buckets[i]) {
            free(buckets[i]);
        }
    }
}

static void* memdup(const void* src, size_t size)
{
    void* dup = malloc(size);
    memcpy(dup, src, size);
    return dup;
}

struct map_t map_create(const size_t key_size, const size_t value_size)
{
    struct map_t map;
    map.indices = NULL;
    map.keys = NULL;
    map.values = NULL;
    map.key_bytes = key_size + !key_size;
    map.value_bytes = value_size + !value_size;
    map.size = 0;
    map.mod = 0;
    map.func = &map_hash_cstr;
    return map;
}

struct map_t map_reserve(const size_t key_size, const size_t value_size, const size_t reserve)
{
    struct map_t map;
    map.key_bytes = key_size + !key_size;
    map.value_bytes = value_size + !value_size;
    map.indices = reserve ? calloc(reserve, sizeof(bucket_t)) : NULL;
    map.keys = reserve ? malloc(reserve * map.key_bytes) : NULL;
    map.values = reserve ? malloc(reserve * map.value_bytes) : NULL;
    map.mod = reserve;
    map.size = 0;
    map.func = &map_hash_cstr;
    return map;
}

struct map_t map_copy(const struct map_t* map)
{
    struct map_t m = *map;
    if (map->mod) {
        size_t i, size;

        m.keys = memdup(map->keys, map->mod * map->key_bytes);
        m.values = memdup(map->values, map->mod * map->value_bytes);
        m.indices = memdup(map->indices, map->mod * sizeof(bucket_t));
        
        for (i = 0; i < map->mod; ++i) {
            size = bucket_size(map->indices[i]);
            if (size) {
                size += BUCKET_DATA_INDEX;
                m.indices[i] = memdup(map->indices[i], size * sizeof(index_t));
            }
        }
    }
    return m;
}

size_t map_capacity(const struct map_t* map)
{
    return map->mod;
}

void map_overload(struct map_t* map, size_t (*func)(const void*))
{
    map->func = func;
}

void* map_key_at(const struct map_t* map, const size_t index)
{
    return _map_key_at(map, index);
}

void* map_value_at(const struct map_t* map, const size_t index)
{
    return _map_value_at(map, index);
}

size_t map_size(const struct map_t*map)
{
    return map->size;
}

size_t map_key_bytes(const struct map_t* map)
{
    return map->key_bytes;
}

size_t map_value_bytes(const struct map_t* map)
{
    return map->value_bytes;
}

index_t map_search(const struct map_t* map, const void* data)
{
    if (map->mod) {
    
        size_t i;
        const size_t hash = map->func(data);
        bucket_t bucket = map->indices[hash % map->mod];
    
        const size_t size = bucket_size(bucket) + BUCKET_DATA_INDEX;
        for (i = BUCKET_DATA_INDEX; i < size; ++i) {
            void* k = _map_key_at(map, bucket[i]);
            if (hash == map->func(k)) {
                return bucket[i] + 1;
            }
        }
    }
    
    return 0;
}

index_t* map_search_all(const struct map_t* map, const void* key)
{
    index_t* found = NULL;
    
    if (map->mod) {

        size_t i, count = 0;
        const size_t hash = map->func(key);
        bucket_t bucket = map->indices[hash % map->mod];

        const size_t size = bucket_size(bucket) + BUCKET_DATA_INDEX;
        index_t indices[0xff];

        for (i = BUCKET_DATA_INDEX; i < size; ++i) {
            void* k = _map_key_at(map, bucket[i]);
            if (hash == map->func(k)) {
                indices[count++] = bucket[i] + 1;
            }
        }

        if (count) {
            const size_t elements = count;
            found = malloc((elements + 1) * sizeof(index_t));
            memcpy(found, indices, elements * sizeof(index_t));
            found[elements] = 0;
        }
    }

    return found;
}

void map_resize(struct map_t* map, const size_t new_size)
{
    size_t i;
    const char* key;
    const size_t size = map->size;
    const size_t bytes = map->key_bytes;

    if (map->mod) {
        buckets_free(map->indices, map->mod);
    }

    map->mod = new_size + !new_size * MAP_SIZE;
    map->keys = realloc(map->keys, map->mod * map->key_bytes);
    map->values = realloc(map->values, map->mod * map->value_bytes);
    map->indices = realloc(map->indices, map->mod * sizeof(bucket_t));
    memset(map->indices, 0, map->mod * sizeof(bucket_t));
    
    key = map->keys;
    for (i = 0; i < size; ++i, key += bytes) {
        const size_t hash_mod = map->func(key) % map->mod;
        map->indices[hash_mod] = bucket_push(map->indices[hash_mod], i);
    }
}

void map_remove(struct map_t* map, const void*key)
{
    if (map->mod) {

        size_t i, search = 0;
        const size_t hash = map->func(key);
        bucket_t bucket = map->indices[hash % map->mod];      

        const size_t size = bucket_size(bucket) + BUCKET_DATA_INDEX;
        for (i = BUCKET_DATA_INDEX; i < size; ++i) {
            void* k = _map_key_at(map, bucket[i]);
            if (hash == map->func(k)) {
                search = i;
                break;
            }
        }
        
        if (search) {
            const index_t find = bucket[search];
            
            char* key = _map_key_at(map, find);
            char* val = _map_value_at(map, find);
            memmove(key, key + map->key_bytes, (map->size - find - 1) * map->key_bytes);
            memmove(val, val + map->value_bytes, (map->size - find - 1) * map->value_bytes);
            
            bucket_remove(bucket, search);
            buckets_reindex(map->indices, map->mod, find);
            --map->size;
        }
    }
}

void map_push(struct map_t* map, const void* key, const void* value)
{
    size_t hash_mod;

    if (map->size == map->mod) {
        map_resize(map, map->mod * 2);
    }

    hash_mod = map->func(key) % map->mod;
    map->indices[hash_mod] = bucket_push(map->indices[hash_mod], map->size);

    memcpy(_map_key_at(map, map->size), key, map->key_bytes);
    memcpy(_map_value_at(map, map->size), value, map->value_bytes);
    ++map->size;
}

size_t map_push_if(struct map_t* map, const void* key, const void* value)
{
    const size_t index = map_search(map, key);
    if (index) {
        return index;
    }

    map_push(map, key, value);
    return 0;
}

void map_free(struct map_t* map)
{
    if (map->indices) {
        buckets_free(map->indices, map->mod);
        free(map->indices);
        free(map->keys);
        free(map->values);

        map->indices = NULL;
        map->keys = NULL;
        map->values = NULL;
        map->size = 0;
        map->mod = 0;
    }
}
