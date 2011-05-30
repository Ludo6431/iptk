#include <stdlib.h>
#include <stdio.h>
#include <string.h>     // strerror, memset
#include <unistd.h>     // open
#include <fcntl.h>      // read
#include <sys/stat.h>   // stat
#include <errno.h>      // errno
#include <sys/ioctl.h>  // ioctl

#include "v4l2_wrapper.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static int xioctl(int fd, int request, void *arg) {
    int r;

    do r = ioctl(fd, request, arg);
    while (r<0 && EINTR==errno);

    return r;
}

int video_open(char *dev_name) {
    int fd;
    struct stat st; 
    struct v4l2_capability cap;

    // open device
    if(stat(dev_name, &st)<0) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if(fd<0) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror (errno));
        exit(EXIT_FAILURE);
    }

    // init device
    if(xioctl(fd, VIDIOC_QUERYCAP, &cap)<0) {
        if (EINVAL==errno) {
            fprintf(stderr, "%s is no V4L2 device\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "VIDIOC_QUERYCAP\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("driver:%s v%u.%u.%u\ncard:%s\nbus_info:%s\n", cap.driver,  (cap.version >> 16) & 0xFF,  (cap.version >> 8) & 0xFF,  (cap.version >> 0) & 0xFF, cap.card, cap.bus_info);

    if(!(cap.capabilities&V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    if(!(cap.capabilities&V4L2_CAP_READWRITE)) {
        fprintf(stderr, "%s does not support read i/o\n", dev_name);
        exit(EXIT_FAILURE);
    }

    return fd;
}

void *video_init(int fd, struct v4l2_format *fmt, unsigned int *size) {
    unsigned int min;
    void *buffer;

    /* try to set format */
    if(xioctl(fd, VIDIOC_S_FMT, fmt)<0) {
        fprintf(stderr, "VIDIOC_S_FMT\n");
        exit(EXIT_FAILURE);
    }

    /* Buggy driver paranoia. */
    min = fmt->fmt.pix.width * 2;
    if(fmt->fmt.pix.bytesperline<min)
        fmt->fmt.pix.bytesperline = min;
    min = fmt->fmt.pix.bytesperline*fmt->fmt.pix.height;
    if(fmt->fmt.pix.sizeimage<min)
        fmt->fmt.pix.sizeimage = min;

    /* allocate memory to store a pic */
    if(size)
        *size = fmt->fmt.pix.sizeimage;
    buffer = malloc(fmt->fmt.pix.sizeimage);
    if(!buffer) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

int video_read(int fd, void *buffer, unsigned int size) {
    fd_set fds;
    struct timeval tv;
    int r;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* Timeout. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select(fd + 1, &fds, NULL, NULL, &tv);
    if(r<0) {
        fprintf(stderr, "select error\n");
        exit(EXIT_FAILURE);
    }
    if(!r) {
        fprintf(stderr, "select timeout\n");
        exit(EXIT_FAILURE);
    }

    r = read(fd, buffer, size);
    if(r<0) {
        switch(errno) {
        case EAGAIN:
            return 0;
        case EIO:
            /* Could ignore EIO, see spec. */

            /* fall through */
        default:
            fprintf(stderr, "read error\n");
            exit(EXIT_FAILURE);
        }
    }

    return r;
}

