#include <stdio.h>
#include <string.h>
#include <ctype.h>

void clean_stdin(void) {
    int c;

    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

char *read_stdin(char * str, size_t size) {
    char *result = fgets(str, size, stdin);

    if (result != NULL) {
        char *lf = strchr(str, '\n'); /* On cherche le caractere '\n'. */

        if (lf != NULL) {
            *lf = '\0';
        }
    }

    return result;
}
