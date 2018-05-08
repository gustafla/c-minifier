#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stb_c_lexer.h"
#include "str_hashmap.h"

#define BLACKLIST_FILENAME ".minify_blacklist"

size_t read_file_to_str(char **dst, char const *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    *dst = (char*)malloc(len);
    fseek(file, 0, SEEK_SET);

    size_t read = fread(*dst, sizeof(char), len, file);
    fclose(file);

    if (read != len) {
        free(*dst);
        fprintf(stderr, "Failed to read file %s\n", filename);
        return 0;
    }
    return len;
}

struct str_hashmap load_blacklist() {
    char *list;
    size_t len = read_file_to_str(&list, BLACKLIST_FILENAME);
    if (!len) {
        return str_hashmap_init(0);
    }

    struct str_hashmap map = str_hashmap_init(128);

    for (char *token = strtok(list, "\n"); token; token = strtok(NULL, "\n")) {
        if (*token != '#' && *token != '\n') { // ignore comment or empty
            str_hashmap_put(&map, token, "");
        }
    }

    return map;
}

/*void gen_alias(char* id, size_t len) {
    static struct str_hashmap ids = str_hashmap_init(2048);
    static unsigned long current = 0;
    static const char *PRINTABLES = "abcdefghijklmnopqrstuvwxyz";

    char *known = str_hashmap_get(&ids, id);
    size_t gen_len;
    if (known) {
        strncpy(id, known, len);
        gen_len = strlen(known);
    } else {
        
    }

    // fill rest of transformed id with whitespace
    for (size_t i = gen_len; i<len; i++) {
        id[i] = ' ';
    }
}*/

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    struct str_hashmap blacklist_map = load_blacklist();

    char *src, *tmp;
    size_t len = read_file_to_str(&src, argv[1]);
    if (!len) {
        return EXIT_FAILURE;
    }
    tmp = (char*)malloc(len);

    stb_lexer lex;
    stb_c_lexer_init(&lex, src, src+len, tmp, len);
    while (stb_c_lexer_get_token(&lex)) {
        if (lex.token == CLEX_id) {
            int len = strlen(lex.string);
            char *test = (char*)malloc(len+1);
            strncpy(test, lex.parse_point-len, len);
            test[len] = '\0';
            if (!str_hashmap_get(&blacklist_map, test)) {
                printf("%s\n", test);
            }
            free(test);
        }
    }

    free(src);
    free(tmp);

    return EXIT_SUCCESS;
}
