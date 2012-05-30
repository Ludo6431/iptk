#ifndef _TOOLS_H
#define _TOOLS_H

#ifdef DEBUG
#include <stdarg.h>
#define mexit(a, b...) _mexit(a, b...)
#else
#define mexit(a, b...) exit(a)
#endif

#ifndef MAX
#   define MAX(a, b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#   define MIN(a, b) ((a)>(b)?(b):(a))
#endif

#ifndef MAX3
#   define MAX3(a, b, c) MAX(a, MAX(b, c))
#endif

#ifndef MIN3
#   define MIN3(a, b, c) MIN(a, MIN(b, c))
#endif

void _mexit(int code, char *format, ...);
inline void *memdup(void *data, unsigned int size);

#endif

