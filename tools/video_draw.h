#ifndef _VIDEO_DRAW_H
#define _VIDEO_DRAW_H

void video_draw_pixel(unsigned char *rgb, unsigned int rowstride, unsigned int h, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
void video_draw_line(unsigned char *rgb, unsigned int rowstride, unsigned int h, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char r, unsigned char g, unsigned char b);
void video_draw_cross(unsigned char *rgb, unsigned int rowstride, unsigned int h, unsigned int x, unsigned int y, unsigned int size, unsigned char r, unsigned char g, unsigned char b);
void video_draw_circle(unsigned char *rgb, unsigned int rowstride, unsigned int h, unsigned int xm, unsigned int ym, unsigned int ray, unsigned char r, unsigned char g, unsigned char b);

void video_draw_box_ovr(unsigned char *rgb, unsigned int w, unsigned int h, unsigned int rowstride, int bx, int by, int bw, int bh, unsigned char r, unsigned char g, unsigned char b);

#endif

