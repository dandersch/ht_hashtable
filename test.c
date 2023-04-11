#include <stdio.h>
#include <cstdlib>

#include <assert.h>

#define HASH_TABLE_IMPLEMENTATION
#include "hash_table.h"

struct foo_t
{
        float bar;
        char* c;
        bool  d;
};

int main()
{
	hash_table_t* table = hash_table_init(&malloc, 4096, sizeof(foo_t));
    assert(table);

    {
	    foo_t entry = {3.1, nullptr, true};
        ht_add_entry(table, "my_entry", &entry);
    }
    foo_t* f = (foo_t*) ht_get_entry(table, "my_entry");
    assert(f);
    printf("%f\n", f->bar);
    printf("%b\n", f->d);

	my_thing_t first = {69, 4.20, true, "hello"};
	my_thing_t second = {13, 3.7, false, "world"};

	hash_table_add_entry("first", first);
	hash_table_add_entry("second", second);

    assert(hash_table_get_entry("first"));
    assert(hash_table_get_entry("second"));
    assert(!hash_table_get_entry("third"));

  	my_thing_t provoke   = {13, 3.7, false, "COLLISION"};
	my_thing_t collision = {13, 3.7, false, "RESOLVED"};
	hash_table_add_entry("aBcD", provoke);
	hash_table_add_entry("DcaB", collision);

    assert(hash_table_get_entry("aBcD"));
    assert(hash_table_get_entry("DcaB"));

	my_thing_t test   = {13, 3.7, false, "TEST"};
	my_thing_t edge   = {13, 3.7, false, "EDGE"};
	hash_table_add_entry("dddddddddddddddddddddddddddddddddddddddd2-" /* 4095 */, test);
	hash_table_add_entry("dddddddddddddddddddddddddddddddddddddddd-2" /* 4095 */, edge);

    assert(hash_table_get_entry("dddddddddddddddddddddddddddddddddddddddd2-"));
    assert(hash_table_get_entry("dddddddddddddddddddddddddddddddddddddddd-2"));

	my_thing_t overflow = {13, 3.7, false, "OVERFLOW"};
	hash_table_add_entry("dddddddddddddddddddddddddddddddddddddddd2." /* 4096 */, overflow);
    assert(hash_table_get_entry("dddddddddddddddddddddddddddddddddddddddd2."));

	my_thing_t overflow2 = {13, 3.7, false, "OVERFLOW2"};
	hash_table_add_entry("dddddddddddddddddddddddddddddddddddddddd2.aksj", overflow2);
    assert(hash_table_get_entry("dddddddddddddddddddddddddddddddddddddddd2.aksj"));

	return 0;
}

