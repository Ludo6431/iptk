#include <stdlib.h>
#include <stdio.h>
#include <string.h>     // strerror, memset
#include <unistd.h>     // open
#include <fcntl.h>      // read
#include <sys/stat.h>   // stat
#include <errno.h>      // errno
#include <sys/ioctl.h>  // ioctl
#include "libv4lconvert.h"

#include "v4l2_wrapper.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static int xioctl(int fd, int request, void *arg) {
    int r;

    do r = ioctl(fd, request, arg);
    while (r<0 && EINTR==errno);

    return r;
}

// FIXME return 0;-1 and set errno
int video_open(struct video_t *vid, char *dev_name) {
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

    vid->fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if(vid->fd<0) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror (errno));
        exit(EXIT_FAILURE);
    }

    // init device
    if(xioctl(vid->fd, VIDIOC_QUERYCAP, &cap)<0) {
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

    return 0;
}

// FIXME return 0;-1 and set errno
int video_config(struct video_t *vid, struct v4l2_format *fmt) {
    // fd      : the file descriptor of the video device
    // src_fmt : will be set to the format the closer to what we want in output
    // fmt     : must be set with the format we want
    int ret;

    // setup convert
    vid->convert_data = v4lconvert_create(vid->fd);
    if (vid->convert_data == NULL)
        exit(1);//, "v4lconvert_create");  // FIXME errno
    if (v4lconvert_try_format(vid->convert_data, fmt, &vid->src_fmt) != 0)
        exit(1);//, "v4lconvert_try_format");  // FIXME errno
    ret = xioctl(vid->fd, VIDIOC_S_FMT, &vid->src_fmt);
    if(ret<0)
        exit(1);    // FIXME fail

#ifdef DEBUG
    printf("raw pixfmt: %c%c%c%c %dx%d\n",
    vid->src_fmt.fmt.pix.pixelformat & 0xff,
    (vid->src_fmt.fmt.pix.pixelformat >> 8) & 0xff,
    (vid->src_fmt.fmt.pix.pixelformat >> 16) & 0xff,
    (vid->src_fmt.fmt.pix.pixelformat >> 24) & 0xff,
    vid->src_fmt.fmt.pix.width, vid->src_fmt.fmt.pix.height);
#endif

    // allocate space for a raw image
    vid->raw_buffer = malloc(vid->src_fmt.fmt.pix.sizeimage);
    if(!vid->raw_buffer)
        exit(1);    // FIXME out of memory

    // keep the destination format
    memcpy(&vid->vid_fmt, fmt, sizeof(*fmt));

    return 0;
}

// FIXME return 0;-1 and set errno
int video_read(struct video_t *vid, void *buffer) {
    fd_set fds;
    struct timeval tv;
    int r;


// FIXME not necessary in case of io_watch
/*    FD_ZERO(&fds);
    FD_SET(vid->fd, &fds);

    // Timeout.
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    r = select(vid->fd + 1, &fds, NULL, NULL, &tv);
    if(r<0) {
        fprintf(stderr, "select error\n");
        exit(EXIT_FAILURE);
    }
    if(!r) {
        fprintf(stderr, "select timeout\n");
        exit(EXIT_FAILURE);
    }*/
// end of FIXME

    // get raw image data
    r = read(vid->fd, vid->raw_buffer, vid->src_fmt.fmt.pix.sizeimage);
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

    // convert data to desired format
    if (v4lconvert_convert(vid->convert_data, &vid->src_fmt, &vid->vid_fmt,
        vid->raw_buffer, vid->src_fmt.fmt.pix.sizeimage,    // raw data
        buffer, vid->vid_fmt.fmt.pix.sizeimage              // converted data
    ) < 0) {
        if (errno != EAGAIN)
            exit(1);    // FIXME errno_exit("v4l_convert");
        return 1;
    }

    return 0;
}

