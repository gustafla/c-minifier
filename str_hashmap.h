#ifndef STR_HASHMAP_H
#define STR_HASHMAP_H

#include <stddef.h>

#define IDENTIFIER_MAX_LEN (64)

struct str_pair {
    char key[IDENTIFIER_MAX_LEN]; // string acting as the key
    char value[IDENTIFIER_MAX_LEN]; // string acting as the value
};

struct str_pair_list {
    size_t capacity; // number of pairs allocated
    size_t use; // number of pairs in use
    struct str_pair *pairs;
};

struct str_hashmap {
    size_t capacity; // number of pair lists allocated
    size_t use; // number of pair lists in use
    struct str_pair_list *lists;
};

#endif // STR_HASHMAP_H
