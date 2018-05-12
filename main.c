#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stb_c_lexer.h"
#include "str_hashmap.h"

#define BLACKLIST_FILENAME ".minify_blacklist"
#define PRINTABLES "abcdefghijklmnopqrstuvwxyz"

size_t read_file_to_str(char **dst, char const *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    *dst = (char*)malloc(len+1);
    fseek(file, 0, SEEK_SET);

    size_t read = fread(*dst, sizeof(char), len, file);
    fclose(file);

    (*dst)[len] = '\0';

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
            printf("%s", token);
            str_hashmap_put(&map, token, "");
        }
    }

    return map;
}

void gen_alias(char* id, size_t len) {
    static struct str_hashmap ids = {0};
    static unsigned long current = 0; // current "number" (to become a base 26)
    if (!ids.capacity) { // init known ids hashmap if not done yet
        ids = str_hashmap_init(2048);
    }

    if (len <= current/(sizeof(PRINTABLES)-1)+1) {
        return; // don't process id when there is no advantage for doing so
    }

    char *gen_id = str_hashmap_get(&ids, id); // check if id is known
    size_t gen_len; // len of string to be written to param "id"
    if (gen_id) {
        gen_len = strlen(gen_id);
        strncpy(id, gen_id, gen_len); // if id known, we can just write it out
    } else { // else generate a new alias
        char gen_id[32]; // TODO make this safer
        /* Thank mr stackoverflow for helping a brainlet out */
        gen_len = 0; // start from most significant
        unsigned long n = current; // our number
        do {
            // insert the character this digit represents
            gen_id[gen_len++] = PRINTABLES[n % (sizeof(PRINTABLES)-1)];
            // move "magnitude" of n down to the next less significant digit
        } while ((unsigned long)(n /= (sizeof(PRINTABLES)-1)) > 0);
        // next number for next call
        current++;
        str_hashmap_put(&ids, id, gen_id); // save result of this id
        strncpy(id, gen_id, gen_len); // write out
    }

    // fill rest of transformed id with whitespace
    for (size_t i = gen_len; i<len; i++) {
        id[i] = ' ';
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    struct str_hashmap blacklist_map = load_blacklist();

    char *src, *new, *tmp;
    size_t len = read_file_to_str(&src, argv[1]);
    if (!len) {
        return EXIT_FAILURE;
    }
    tmp = (char*)malloc(len); // stb_lexer scratch space

    stb_lexer lex;
    stb_c_lexer_init(&lex, src, src+len, tmp, len);
    while (stb_c_lexer_get_token(&lex)) {
        if (lex.token == CLEX_id) {
            int len = strlen(lex.string);
            if (!str_hashmap_get(&blacklist_map, lex.string)) {
                gen_alias(lex.string, len);
                strncpy(lex.parse_point-len, lex.string, len);
            }
        }
    }

    printf("%s", src);
    
    /*for (int i=0; i<20000000; i++) {
        char tmp[20];
        tmp[19] = '\0';
        gen_alias(tmp, 20);
        printf("%s.\n", tmp);
    }*/

    free(src);
    free(tmp);

    return EXIT_SUCCESS;
}
