#include <stdlib.h>
#include <stdio.h>
#include "stb_c_lexer.h"

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
            test[len] = NULL;
            printf("%s\n", test);
            free(test);
        }
    }

    free(src);
    free(tmp);
    return EXIT_SUCCESS;
}
