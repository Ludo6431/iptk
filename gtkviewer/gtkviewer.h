#ifndef _GTKVIEWER_H
#define _GTKVIEWER_H

#include "cust_params.h"

void    gv_init         (int *pargc, char **pargv[], char *wtitle, char *help);

// medias
typedef void (*gv_destroy)(void *, void *);
int     gv_media_new    (char *name, char *desc, unsigned int width, unsigned int height);
void    gv_media_update (int mid, unsigned char *data, unsigned int width, unsigned int height, gv_destroy destroy, void *destroy_data);
void    gv_media_del    (int mid);

// parameters
int     gv_gparam_new   (char *gname, char *gdesc);
void    gv_gparam_del   (int gid);
void    gv_param_add    (int gid, param_t *p);

void    gv_run          ();

#endif

