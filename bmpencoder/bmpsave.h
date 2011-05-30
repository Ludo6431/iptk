#ifndef _BMPSAVE_H
#define _BMPSAVE_H

#ifndef PACKED
#   define PACKED __attribute__((__packed__))
#endif

#include "ndstypes.h"

typedef struct {
    u16 type;                       /* Magic identifier            */
    u32 size;                       /* File size in bytes          */
    u16 reserved1, reserved2;
    u32 offset;                     /* Offset to image data, bytes */
} PACKED HEADER;

typedef struct {
    u32 size;                       /* Header size in bytes      */
    u32 width,height;               /* Width and height of image */
    u16 planes;                        /* Number of colour planes   */
    u16 bits;                       /* Bits per pixel            */
    u32 compression;                /* Compression type          */
    u32 imagesize;                    /* Image size in bytes       */
    u32 xresolution,yresolution;    /* Pixels per meter          */
    u32 ncolours;                   /* Number of colours         */
    u32 importantcolours;           /* Important colours         */
} PACKED INFOHEADER;

void bmpsave(const char* filename, char *data, unsigned w, unsigned h); // save RGB data 24bit as a bmp file

#endif

