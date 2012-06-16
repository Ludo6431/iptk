#ifndef _CLOCK_H
#define _CLOCK_H

#include <time.h>

#if 1

typedef struct timespec CLOCK_TYPE;

#define CLOCK_INIT(v) do { \
        clock_getres(CLOCK_PROCESS_CPUTIME_ID, &(v)); \
        fprintf(stderr, "CPU time resolution: %lu.%09li\n", v.tv_sec, v.tv_nsec); \
        \
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &(v)); \
    } while(0)

#define CLOCK_STEP(prev, s) do { \
        CLOCK_TYPE cur; \
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cur); \
        if(s) \
            fprintf(stderr, "%s: %lu.%09li\n", (s), cur.tv_sec - (prev).tv_sec, cur.tv_nsec - (prev).tv_nsec); \
        \
        (prev).tv_sec = cur.tv_sec; \
        (prev).tv_nsec = cur.tv_nsec; \
    } while(0)

#else

typedef int CLOCK_TYPE;
#define CLOCK_INIT(v) 
#define CLOCK_STEP(prev, s) 

#endif

#endif

