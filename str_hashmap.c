#include "str_hashmap.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void assert_str(char *str) {
    if (strlen(str) >= IDENTIFIER_MAX_LEN) {
        fprintf(stderr, "too long string: %s\n", str);
        exit(EXIT_FAILURE);
    }
}

// this function gives a basic hash value for a null-terminated string
unsigned long str_hash(unsigned char *str){
    unsigned long hash = 13;
    int c;
    while (c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int str_pair_list_indexof(struct str_pair_list *list, char *key) {
    // key length assertion not needed as function not public
    for (int i=0; i < list->use; i++) {
        if (strcmp(key, list->pairs[i].key) == 0) {
            // if keys match, return index
            return i;
        }
    }
    return -1;
}

void str_pair_list_add(struct str_pair_list *list, char *key, char *value) {
    if (!list->capacity) { // uninitialized, allocate some
        list->pairs = (struct str_pair*)malloc(sizeof(struct str_pair)*2);
        list->capacity = 2;
    }
    if (list->use + 1 > list->capacity) { // can't fit, double the size
        size_t list_size = sizeof(struct str_pair) * list->capacity;
        struct str_pair *new = (struct str_pair*)malloc(list_size * 2);
        memcpy(new, list->pairs, list_size); // shouldn't need to but let's
        free(list->pairs);
        list->pairs = new;
        list->capacity *= 2;
    }
    strncpy(list->pairs[list->use].key, key, IDENTIFIER_MAX_LEN);
    strncpy(list->pairs[list->use].value, value, IDENTIFIER_MAX_LEN);
    list->use++;
}

struct str_hashmap str_hashmap_init(size_t capacity) {
    struct str_hashmap map;

    // allocate and zero pair lists
    size_t lists_size = sizeof(struct str_pair_list) * capacity;
    map.lists = (struct str_pair_list*)malloc(lists_size);
    memset(map.lists, 0, lists_size);

    // set other fields
    map.capacity = capacity;
    map.use = 0;

    return map;
}

char *str_hashmap_get(struct str_hashmap *map, char *key) {
    // make sure we can operate with this key
    assert_str(key);

    // check if hash's list has anything yet
    unsigned long hash = str_hash(key) % map->capacity;
    if (!map->lists[hash].use) { // if list "hit" is uninitialized
        return NULL;
    }

    struct str_pair_list *list_in_index = map->lists + hash;

    // iterate pairs in list and search for mathching keys
    int index = str_pair_list_indexof(list_in_index, key);
    if (index >= 0) {
        return list_in_index->pairs[index].value;
    }

    return NULL;
}

void str_hashmap_put(struct str_hashmap *map, char *key, char *value) {
    // make sure strings are reasonable length
    assert_str(key);
    assert_str(value);

    unsigned long hash = str_hash(key) % map->capacity;

    struct str_pair_list *list_in_index = map->lists + hash;
    int index = str_pair_list_indexof(list_in_index, key);
    if (index < 0) {
        // add new pair to index's list
        str_pair_list_add(list_in_index, key, value);
    } else {
        // change the found pair's value string
        strncpy(list_in_index->pairs[index].value, value, IDENTIFIER_MAX_LEN);
    }
}
