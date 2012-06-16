#ifndef _GV_PARAMS_H
#define _GV_PARAMS_H

#include "param.h"

// group of parameters
int         gv_gparam_new       (char *gname, char *gdesc);
void        gv_gparam_del       (int gid);

// parameter
void        gv_param_add        (int gid, param_t *p);
GtkWidget * gv_param_get_widget (param_t *p);
void        gv_param_add_widget (int gid, GtkWidget *w);

#endif

