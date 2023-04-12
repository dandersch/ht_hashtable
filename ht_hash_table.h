#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t, ...

struct ht_hash_table_t;
typedef struct ht_hash_table_t ht_hash_table_t;

/*
 * NOTES:
 * - key string is not copied when adding entries (pointer must stay valid)
 * - table doesn't realloc when reaching capacity
 * - no way to iterate through all entries
 * - naive hash function
 */

ht_hash_table_t* ht_init(void* (*allocator)(size_t),  const uint32_t table_size, const uint32_t entry_size);
int              ht_add_entry(ht_hash_table_t* table, const char* key, void* entry);
void*            ht_get_entry(ht_hash_table_t* table, const char* key);
// TODO int      ht_free_entry(ht_hash_table_t* table, const char* key);
// TODO int      ht_destroy(ht_hash_table_t* table);

/* implementation */
#ifdef HT_HASH_TABLE_IMPLEMENTATION
#include <string.h> // for strlen(), strcmp(), memcpy()

typedef struct ht_entry_t
{
    const char* key;
    void*       entry;
} ht_entry_t;

typedef struct ht_hash_table_t
{
    ht_entry_t* entries;
    void*       (*allocator)(size_t);
    uint32_t    entry_size;
    uint32_t    table_size;
} ht_hash_table_t;

ht_hash_table_t* ht_init(void* (*allocator)(size_t), const uint32_t table_size, const uint32_t entry_size)
{
    assert(!(table_size&(table_size-1))); // size must be power of 2
    uint32_t bytesize   = sizeof(ht_entry_t) * table_size;
    ht_hash_table_t* table = (ht_hash_table_t*) allocator(sizeof(ht_hash_table_t));
    table->allocator    = allocator;
    table->table_size   = table_size;
    table->entry_size   = entry_size;
    table->entries      = (ht_entry_t*) allocator(bytesize);
    memset(table->entries, 0, bytesize);
    return table;
}

static uint32_t ht_hash_function(ht_hash_table_t* table, const char* key)
{ 
    // TODO better hash function
    uint32_t ht_idx = 0;
    for (int i = 0; i < strlen(key); i++) // unsecure
    {
        ht_idx += key[i];
    }

    // mask off high bits so that the index is under the hash table size
    ht_idx &= (table->table_size / table->entry_size) - 1;

    return ht_idx;
}

int ht_add_entry(ht_hash_table_t* table, const char* key, void* entry)
{
    uint32_t hash = ht_hash_function(table, key);
    int wrapped_around = 0;

    // collision handling w/ internal chaining
    for (uint32_t idx = hash; idx < table->table_size; idx++)
    {
        if (table->entries[idx].key == NULL) // no collision
        {
            table->entries[idx].entry = (void*) table->allocator(table->entry_size);
            memcpy(table->entries[idx].entry, entry, table->entry_size);
            table->entries[idx].key = key;
            return 1;
        }

        // array is completely filled
        if (idx == hash && wrapped_around) { return 0; }

        if (idx == (table->table_size - 1)) // handle wraparound
        {
            wrapped_around = 1;
            idx = 0;
        }
    }

    return 0;
}

void* ht_get_entry(ht_hash_table_t* table, const char* key)
{
    uint32_t hash       = ht_hash_function(table, key);
    int wrapped_around = 0;

    for (uint32_t idx = hash; idx < table->table_size; idx++)
    {
        if (table->entries[idx].key == NULL) // we early out if there is no key
        {
            return NULL;
        }

        if (strcmp(table->entries[idx].key, key) == 0) // key found
        {
            return table->entries[idx].entry;
        }

        if (idx == hash && wrapped_around) // array completely searched
        {
            return NULL;
        }

        if (idx == (table->table_size - 1)) // handle wraparound
        {
            wrapped_around = 1;
            idx = 0;
        }
    }

    return NULL;
}
#endif