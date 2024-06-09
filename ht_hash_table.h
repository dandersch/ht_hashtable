#pragma once

#include <stddef.h>  /* for size_t */

struct ht_hash_table_t;
typedef struct ht_hash_table_t ht_hash_table_t;

/*
 * NOTES:
 * - key string is not copied when adding entries (pointer must stay valid)
 * - table doesn't realloc when reaching capacity
 * - no way to iterate through all entries
 * - naive hash function
 */

ht_hash_table_t* ht_init(void* (*allocator)(size_t),  const size_t table_size, const size_t entry_size);
int              ht_add_entry(ht_hash_table_t* table, const char* key, void* entry);
void*            ht_get_entry(ht_hash_table_t* table, const char* key);
int              ht_free_entry(ht_hash_table_t* table, const char* key, void (*deallocator)(void*));
void             ht_destroy(ht_hash_table_t** table, void (*deallocator)(void*));

#ifndef HT_ASSERT
  #include <assert.h>
  #define HT_ASSERT(expr) assert(expr)
#endif

#ifndef HT_LOG
  #include <stdio.h>
  #define HT_LOG(msg, ...) printf(msg, __VA_ARGS__)
#endif

#ifndef HT_LOG_COLLISIONS
  #define HT_LOG_COLLISIONS 0
#endif

/* implementation */
#ifdef HT_HASH_TABLE_IMPLEMENTATION
#include <string.h> /* for strlen(), strcmp(), memcpy() */

typedef struct ht_entry_t
{
    const char* key;
    void*       entry;
} ht_entry_t;

typedef struct ht_hash_table_t
{
    ht_entry_t* entries;
    void*       (*allocator)(size_t);
    size_t      entry_size;
    size_t      table_size;
} ht_hash_table_t;

ht_hash_table_t* ht_init(void* (*allocator)(size_t), const size_t table_size, const size_t entry_size)
{
    HT_ASSERT(!(table_size&(table_size-1))); /* size must be power of 2 */

    size_t bytesize        = sizeof(ht_entry_t) * table_size;
    ht_hash_table_t* table = (ht_hash_table_t*) allocator(sizeof(ht_hash_table_t));
    table->allocator       = allocator;
    table->table_size      = table_size;
    table->entry_size      = entry_size;
    table->entries         = (ht_entry_t*) allocator(bytesize);
    memset(table->entries, 0, bytesize);
    return table;
}

static size_t hash_function(ht_hash_table_t* table, const char* key)
{ 
    /* TODO better hash function */
    size_t ht_idx = 0;
    int i = 0;
    for (i = 0; i < strlen(key); i++) /* unsecure */
    {
        ht_idx += key[i];
    }

    /* mask off high bits so that the index is under the hash table size */
    ht_idx &= (table->table_size / table->entry_size) - 1;

    return ht_idx;
}

int ht_add_entry(ht_hash_table_t* table, const char* key, void* entry)
{
    size_t hash = hash_function(table, key);
    int wrapped_around = 0;

    /* collision handling w/ internal chaining */
    size_t idx = hash;
    for (idx = hash; idx < table->table_size; idx++)
    {
        if (table->entries[idx].key == NULL) /* no collision */
        {
            if (idx != hash && (HT_LOG_COLLISIONS)) { HT_LOG("Hash collision for key: %s\n", key); }

            table->entries[idx].entry = (void*) table->allocator(table->entry_size);
            memcpy(table->entries[idx].entry, entry, table->entry_size);
            table->entries[idx].key = key;
            return 1;
        }

        /* array is completely filled */
        if (idx == hash && wrapped_around)
        {
            HT_LOG("Hash table with capacity %zu reached capacity\n", table->table_size);
            return 0;
        }

        if (idx == (table->table_size - 1)) /* handle wraparound */
        {
            wrapped_around = 1;
            idx = 0;
        }
    }

    return 0;
}

typedef struct index_or_error_t { size_t index; int error; } index_or_error_t;
index_or_error_t get_index_for_key(ht_hash_table_t* table, const char* key)
{
    index_or_error_t index_or_error;
    index_or_error.index = 0;
    index_or_error.error = 0;
    size_t hash  = hash_function(table, key);
    int wrapped_around = 0;

    size_t idx;
    for (idx = hash; idx < table->table_size; idx++)
    {
        if (table->entries[idx].key == NULL) /* we early out if there is no key */
        {
            index_or_error.error = 1;
            break;
        }

        if (strcmp(table->entries[idx].key, key) == 0) /* key found */
        {
            index_or_error.index = idx;
            break;
        }

        if (idx == hash && wrapped_around) /* array completely searched */
        {
            index_or_error.error = 1;
            break;
        }

        if (idx == (table->table_size - 1)) /* handle wraparound */
        {
            wrapped_around = 1;
            idx = 0;
        }
    }

    return index_or_error;
}

void* ht_get_entry(ht_hash_table_t* table, const char* key)
{
    index_or_error_t index_or_error = get_index_for_key(table, key);

    if (index_or_error.error)
    {
        return NULL;
    }
    else
    {
        return table->entries[index_or_error.index].entry;
    }
}

int ht_free_entry(ht_hash_table_t* table, const char* key, void (*deallocator)(void*))
{
    index_or_error_t index_or_error = get_index_for_key(table, key);

    if (index_or_error.error)
    {
        return 0;
    }
    else
    {
        deallocator(table->entries[index_or_error.index].entry);
        table->entries[index_or_error.index].key = NULL;
        return 1;
    }
}

void  ht_destroy(ht_hash_table_t** table, void (*deallocator)(void*))
{
    HT_ASSERT(table);

    deallocator((*table)->entries);
    (*table)->entries = NULL;
    deallocator(*table);
    (*table) = NULL;
}
#endif
