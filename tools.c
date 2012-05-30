#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "outils.h"

void _mexit(int code, char *format, ...) {
    if(format) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
        va_end(ap);
    }

    exit(code);
}

inline void *memdup(void *data, unsigned int size) {
    void *ret = malloc(size);
    if(ret)
        memcpy(ret, data, size);

    return ret;
}

