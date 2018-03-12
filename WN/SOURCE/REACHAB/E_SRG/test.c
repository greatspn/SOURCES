#include <stdio.h>
#include <stdarg.h>

int fdscanf(FILE *f,  const char *format, ...) {
    va_list args;
    int rc;
    va_start(args, format);
    rc = vfscanf(f, format, args);
    va_end(args);

    return rc;
}

int main() {

    int tab[2] = {0, 0};
    fdscanf(stdin, "%d %d", tab, tab + 1);
    printf("%d %d", tab[0], tab[1]);
}
