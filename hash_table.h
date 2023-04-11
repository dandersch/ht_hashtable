#pragma once

/*        */
/* header */
/*        */
#include <stddef.h>  // for size_t
#include <stdint.h>

struct my_thing_t
{
	int32_t     foo;
	float       bar;
	bool        handled;
	const char* desc;
};

struct hash_table_t;

hash_table_t* hash_table_init(void* (*allocator)(size_t), uint32_t table_size, uint32_t entry_size);
bool          hash_table_add_entry(const char* key, my_thing_t entry);
my_thing_t*   hash_table_get_entry(const char* key, my_thing_t* out_entry);

bool          ht_add_entry(hash_table_t* table, const char* key, void* entry);
void*         ht_get_entry(hash_table_t* table, const char* key);

/*                */
/* implementation */
/*                */
#ifdef HASH_TABLE_IMPLEMENTATION
#include <cstring>

#define HASH_TABLE_SIZE 4096 // must be a power of two because of the masking in the hash function

struct hash_table_entry_t
{
	my_thing_t  entry;
	const char* key;
};

struct ht_entry_t
{
	const char* key;
	void*       entry;
};

struct hash_table_t
{
    ht_entry_t* entries;
    void*       (*allocator)(size_t);
    uint32_t    entry_size;
    uint32_t    table_size;
};

hash_table_entry_t hash_table_backend[HASH_TABLE_SIZE] = {0};

hash_table_t* hash_table_init(void* (*allocator)(size_t), uint32_t table_size, uint32_t entry_size)
{
    // assert( size is power of 2 );
    uint32_t bytesize   = sizeof(ht_entry_t) * table_size;
    hash_table_t* table = (hash_table_t*) allocator(sizeof(hash_table_t));
    table->allocator    = allocator;
    table->table_size   = table_size;
    table->entry_size   = entry_size;
    table->entries      = (ht_entry_t*) allocator(bytesize);
    memset(table->entries, 0, bytesize);
    return table;
}

static uint32_t ht_hash_function(hash_table_t* table, const char* key) // TODO better hash function
{
	uint32_t ht_idx = 0;
	for (int i = 0; i < strlen(key); i++) // unsecure
	{
		ht_idx += key[i];
	}

	// mask off high bits so that the index is under the hash table size
	ht_idx &= (table->table_size / table->entry_size) - 1;

	return ht_idx;
}
static uint32_t hash_function(const char* key) // TODO better hash function
{
	uint32_t ht_idx = 0;
	for (int i = 0; i < strlen(key); i++) // unsecure
	{
		ht_idx += key[i];
	}

	// mask off high bits so that the index is under the hash table size
	ht_idx &= (sizeof(hash_table_backend)/ sizeof((hash_table_backend)[0])) - 1;

	return ht_idx;
}

bool ht_add_entry(hash_table_t* table, const char* key, void* entry)
{
	uint32_t hash = ht_hash_function(table, key);
	bool wrapped_around = false;

	// collision handling w/ internal chaining
	for (uint32_t idx = hash; idx < table->table_size; idx++)
	{
		if (table->entries[idx].key == NULL) // no collision
		{
			table->entries[idx].entry = (void*) table->allocator(table->entry_size);
			memcpy(table->entries[idx].entry, entry, table->entry_size);
			table->entries[idx].key = key;
			return true;
		}

		// array is completely filled
		if (idx == hash && wrapped_around) { return false; }

		if (idx == (table->table_size - 1)) // handle wraparound
		{
			wrapped_around = true;
			idx = 0;
		}
	}

	return false;
}

bool hash_table_add_entry(const char* key, my_thing_t entry)
{
	uint32_t hash = hash_function(key);
	bool wrapped_around = false;

	// collision handling w/ internal chaining
	for (uint32_t idx = hash; idx < HASH_TABLE_SIZE; idx++)
	{
		if (hash_table_backend[idx].key == NULL) // no collision
		{
			hash_table_backend[idx] = {entry, key};
			return true;
		}

		// array is completely filled
		if (idx == hash && wrapped_around) { return false; }

		if (idx == (HASH_TABLE_SIZE - 1)) // handle wraparound
		{
			wrapped_around = true;
			idx = 0;
		}
	}

	return false;
}

void* ht_get_entry(hash_table_t* table, const char* key)
{
	uint32_t hash       = ht_hash_function(table, key);
	bool wrapped_around = false;

	for (uint32_t idx = hash; idx < table->table_size; idx++)
	{
		if (table->entries[idx].key == NULL) // we early out if there is no key
		{
			return nullptr;
		}

		if (strcmp(table->entries[idx].key, key) == 0) // key found
		{
			return table->entries[idx].entry;
		}

		if (idx == hash && wrapped_around) // array completely searched
		{
			return nullptr;
		}

		if (idx == (table->table_size - 1)) // handle wraparound
		{
			wrapped_around = true;
			idx = 0;
		}
	}

	return nullptr;
}

my_thing_t* hash_table_get_entry(const char* key)
{
	uint32_t hash       = hash_function(key);
	bool wrapped_around = false;

	for (uint32_t idx = hash; idx < HASH_TABLE_SIZE; idx++)
	{
		if (hash_table_backend[idx].key == NULL) // we early out if there is no key
		{
			return nullptr;
		}

		if (strcmp(hash_table_backend[idx].key, key) == 0) // key found
		{
			return &hash_table_backend[idx].entry;
		}

		if (idx == hash && wrapped_around) // array completely searched
		{
			return nullptr;
		}

		if (idx == (HASH_TABLE_SIZE - 1)) // handle wraparound
		{
			wrapped_around = true;
			idx = 0;
		}
	}

	return nullptr;
}
#endif
