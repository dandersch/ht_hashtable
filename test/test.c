#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HT_HASH_TABLE_IMPLEMENTATION
#include "../ht_hash_table.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct foo_t
{
    float bar;
    char* c;
    bool  d;
} foo_t;

typedef struct my_thing_t
{
    int32_t     foo;
    float       bar;
    bool        handled;
    const char* desc;
} my_thing_t;

int main()
{
    ht_hash_table_t* table = ht_init(&malloc, 4096, sizeof(foo_t));
    assert(table);

    {
        foo_t entry = {3.1, NULL, true};
        ht_add_entry(table, "my_entry", &entry);
    }
    foo_t* f = (foo_t*) ht_get_entry(table, "my_entry");
    assert(f);
    assert(f->bar == 3.1f);
    assert(f->d);

    ht_hash_table_t* ht = ht_init(&malloc, 4096, sizeof(my_thing_t));
    assert(ht);

    my_thing_t first = {69, 4.20, true, "hello"};
    my_thing_t second = {13, 3.7, false, "world"};
    ht_add_entry(ht, "first",  &first);
    ht_add_entry(ht, "second", &second);

    assert(ht_get_entry(ht,  "first"));
    assert(ht_get_entry(ht,  "second"));
    assert(!ht_get_entry(ht, "third"));

    my_thing_t provoke   = {13, 3.7, false, "COLLISION"};
    my_thing_t collision = {13, 3.7, false, "RESOLVED"};
    ht_add_entry(ht, "aBcD", &provoke);
    ht_add_entry(ht, "DcaB", &collision);

    assert(ht_get_entry(ht, "aBcD"));
    assert(ht_get_entry(ht, "DcaB"));

    my_thing_t test   = {13, 3.7, false, "TEST"};
    my_thing_t edge   = {13, 3.7, false, "EDGE"};
    ht_add_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2-" /* 4095 */, &test);
    ht_add_entry(ht, "dddddddddddddddddddddddddddddddddddddddd-2" /* 4095 */, &edge);

    assert(ht_get_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2-"));
    assert(ht_get_entry(ht, "dddddddddddddddddddddddddddddddddddddddd-2"));

    my_thing_t overflow = {13, 3.7, false, "OVERFLOW"};
    ht_add_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2." /* 4096 */, &overflow);
    assert(ht_get_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2."));

    my_thing_t overflow2 = {13, 3.7, false, "OVERFLOW2"};
    ht_add_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2.aksj", &overflow2);
    assert(ht_get_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2.aksj"));

    int freed = ht_free_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2.aksj", &free);
    assert(freed);
    assert(!ht_get_entry(ht, "dddddddddddddddddddddddddddddddddddddddd2.aksj"));

    freed = ht_free_entry(ht, "random", &free);
    assert(!freed);

    ht_destroy(&table, &free);
    assert(!table);
    ht_destroy(&ht, &free);
    assert(!ht);

    return 0;
}

