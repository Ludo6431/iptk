#ifndef _PARAMETERS_H
#define _PARAMETERS_H

// define parameters types
typedef enum {
    PT_NONE,
    PT_BOOL,
    PT_INT
} ptype_t;

struct _pt_bool {
    // TODO: YESNO or TRUEFALSE ?
};

struct _pt_int {
    int min, max, step;
};

typedef struct {
    char *name;
    char *desc;

    ptype_t type;

    volatile void *val;

    union {
        struct _pt_bool pt_bool;

        struct _pt_int pt_int;

// TODO add menu, ...
    };
} param_t;

void            param_init      (param_t *p, char *name, char *desc, ptype_t type, volatile void *val, ...);

inline void *   param_get       (param_t *p);
inline void     param_set       (param_t *p, void *val);
void            param_get_extra (param_t *p, ...);
void            param_set_extra (param_t *p, ...);

void            param_free      (param_t *p);

#endif

