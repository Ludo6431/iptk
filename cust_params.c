#include <stdlib.h>
#include <stdarg.h> // va_*
#include <assert.h> // assert
#include <string.h> // strdup

#include "cust_params.h"

void param_init(param_t *p, char *name, char *desc, ptype_t type, volatile void *val, ...) {
    va_list ap;

    assert(p);

    if(name)
        p->name = strdup(name);
    if(desc)
        p->desc = strdup(desc);
    p->type = type;
    p->val = val;

    va_start(ap, val);
    switch(p->type) {
    case PT_INT:
        p->pt_int.min = va_arg(ap, int);
        p->pt_int.max = va_arg(ap, int);
        p->pt_int.step = va_arg(ap, int);
        break;
    default:
        break;
    }
    va_end(ap);
}

inline void *param_get(param_t *p) {
    assert(p);

    return p->val;
}

inline void param_set(param_t *p, void *val) {
    assert(p);

    p->val = val;    
}

void param_get_extra(param_t *p, ...) {
    va_list ap;

    assert(p);

    va_start(ap, p);
    switch(p->type) {
    case PT_INT:
        *va_arg(ap, int *) = p->pt_int.min;
        *va_arg(ap, int *) = p->pt_int.max;
        *va_arg(ap, int *) = p->pt_int.step;
        break;
    default:
        break;
    }
    va_end(ap);
}

void param_set_extra(param_t *p, ...) {
    va_list ap;

    assert(p);

    va_start(ap, p);
    switch(p->type) {
    case PT_INT:
        p->pt_int.min = va_arg(ap, int);
        p->pt_int.max = va_arg(ap, int);
        p->pt_int.step = va_arg(ap, int);
        break;
    default:
        break;
    }
    va_end(ap);
}

void param_free(param_t *p) {
    assert(p);

    if(p->name)
        free(p->name);
    if(p->desc)
        free(p->desc);
    p->type = PT_NONE;
}

