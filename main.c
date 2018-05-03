#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stb_c_lexer.h"

struct str_pair {
    char *key;
    char *val;
};

struct str_hashmap {
    size_t capacity;
    size_t len;
    struct str_pair *pairs;
};

struct str_hashmap str_hashmap_init(size_t capacity) {
    struct str_hashmap m;
    m.pairs = (struct str_pair*)malloc(sizeof(struct str_pair) * capacity);
    m.capacity = capacity;
    m.len = 0;
    return m;
}

unsigned long str_hash(unsigned char *str){
    unsigned long hash = 13;
    int c;
    while (c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    char *src = (char*)malloc(len);
    char *tmp = (char*)malloc(len);
    fseek(file, 0, SEEK_SET);

    fread(src, sizeof(char), len, file);
    fclose(file);

    stb_lexer lex;
    stb_c_lexer_init(&lex, src, src+len, tmp, len);
    while (stb_c_lexer_get_token(&lex)) {
        if (lex.token == CLEX_id) {
            int len = strlen(lex.string);
            char *test = (char*)malloc(len+1);
            strncpy(test, lex.parse_point-len, len);
            test[len] = '\0';
            printf("%s\n", test);
            free(test);
        }
    }

    free(src);
    free(tmp);
    return EXIT_SUCCESS;
}
