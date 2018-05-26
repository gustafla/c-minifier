#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stb_c_lexer.h"
#include "str_hashmap.h"
#include "config.h"

#define BLACKLIST_FILENAME ".minify_blacklist"
#define PRINTABLES "abcdefghijklmnopqrstuvwxyz"

struct str_hashmap g_ids_map; // will record all ids

size_t read_file_to_str(char **dst, char const *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    if (!len) {
        return 0; // Don't read and malloc an empty file
    }
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

void gen_alias(char* id, size_t len) {
    static struct str_hashmap ids = {0};
    static unsigned long current = 0; // current "number" (to become a base 26)
    if (!ids.capacity) { // init known ids hashmap if not done yet
        ids = str_hashmap_init(2048);
    }


    char *gen_id = str_hashmap_get(&ids, id); // check if id is known
    size_t gen_len; // len of string to be written to param "id"
    if (gen_id) {
        gen_len = strlen(gen_id);
        strcpy(id, gen_id); // if id known, we can just write it out
    } else if (len < current/(sizeof(PRINTABLES)-1)+1) {
        return;
    } else {
        char gen_id[32]; // TODO make this safer
        do {
            gen_len = 0; // start from most significant
            unsigned long n = current; // our number
            /* Thank mr stackoverflow for helping a brainlet out */
            do {
                // insert the character this digit represents
                gen_id[gen_len++] = PRINTABLES[n % (sizeof(PRINTABLES)-1)];
                // move "magnitude" of n down to the next less significant digit
            } while ((unsigned long)(n /= (sizeof(PRINTABLES)-1)) > 0);
            // next number for next call
            current++;
            gen_id[gen_len] = '\0';
            // try again if id is potentially reserved
        } while (str_hashmap_get(&g_ids_map, gen_id));
        str_hashmap_put(&ids, id, gen_id); // save result of this id
        strncpy(id, gen_id, gen_len); // write out
    }

    // fill rest of transformed id with whitespace
    for (size_t i = gen_len; i<len; i++) {
        id[i] = ' ';
    }
}

void replace_id_with_alias(stb_lexer *lex, struct str_hashmap *blacklist_map) {
    size_t len = strlen(lex->string);
    gen_alias(lex->string, len);
    strncpy(lex->parse_point-len, lex->string, len);
}

void record_id_to_blacklist(stb_lexer *lex, struct str_hashmap *blacklist_map) {
    str_hashmap_put(blacklist_map, lex->string, "");
}

void process(char *src, size_t len, struct str_hashmap *blacklist_map, void(*callback)(stb_lexer*, struct str_hashmap*)) {
    stb_lexer lex;
    char *tmp = (char*)malloc(len); // stb_lexer scratch space

    stb_c_lexer_init(&lex, src, src+len, tmp, len);
    while (stb_c_lexer_get_token(&lex)) {
        if (lex.token == CLEX_id) {
            if (!str_hashmap_get(blacklist_map, lex.string)) {
                callback(&lex, blacklist_map);
            }
        }
    }

    free(tmp);
}

struct str_hashmap load_blacklist(char **header_files) {
    char *str;
    size_t len = read_file_to_str(&str, BLACKLIST_FILENAME);
    struct str_hashmap map = str_hashmap_init(128);

    if (len) {
        for (char *token = strtok(str, "\n"); token; token = strtok(NULL, "\n")) {
            if (*token != '#' && *token != '\n') { // ignore comment or empty
                str_hashmap_put(&map, token, "");
            }
        }
        free(str);
    } // if read_file_to_str returns 0, should be nothing to free up manually

    for (int i=0; i < sizeof(INTERNAL_BLACKLIST)/sizeof(char*); i++) {
        str_hashmap_put(&map, INTERNAL_BLACKLIST[i], "");
    }

    for (char **f = header_files; *f != NULL; f++) {
        len = read_file_to_str(&str, *f);
        if (!len) {
            exit(EXIT_FAILURE);
        }
        process(str, len, &map, record_id_to_blacklist);
        free(str);
    }

    return map;
}


int main(int argc, char *argv[]) {
    char *filename = NULL;
    size_t header_files_size = sizeof(char*) * argc + 1;
    char **header_files = (char**)malloc(header_files_size);
    size_t header_files_n = 0;
    memset(header_files, 0, header_files_size);

    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "-h expects a filename\n");
                return EXIT_SUCCESS;
            }
            header_files[header_files_n++] = argv[++i];
        } else {
            filename = argv[i];
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "please give an input file\n");
        return EXIT_SUCCESS;
    }

    // Load blacklist
    struct str_hashmap blacklist_map = load_blacklist(header_files);
    free(header_files);

    // Read source file
    char *src;
    size_t len = read_file_to_str(&src, filename);
    if (!len) {
        return EXIT_FAILURE;
    }

    // Pre-pass store all ids (globally, TODO refactor)
    g_ids_map = str_hashmap_init(2048);
    process(src, len, &g_ids_map, record_id_to_blacklist);

    // Actually replace all ids
    process(src, len, &blacklist_map, replace_id_with_alias);
    printf("%s", src);

    free(src);

    return EXIT_SUCCESS;
}
