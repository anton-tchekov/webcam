#ifndef __WEBCAM_H__
#define __WEBCAM_H__

#ifdef __WIN32

typedef struct CAPTURE_PARAMETERS
{
	int *Buffer;
	int Width;
	int Height;
} CaptureParameters;

typedef struct WEBCAM
{
	int DeviceID;
	CaptureParameters Capture;
} Webcam;

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <time.h>

struct buffer
{
	void *start;
	size_t length;
};

typedef struct WEBCAM
{
	int fd, width, height;
	unsigned int n_buffers;
	struct v4l2_buffer buf;
	struct buffer *buffers;
} Webcam;

#endif

int webcam_init(void);
int webcam_open(Webcam *webcam, int device_id, int width, int height);
void webcam_size(Webcam *webcam, int *width, int *height);
char *webcam_name(int device_id, char *name, int len);
unsigned char *webcam_frame(Webcam *webcam);
void webcam_close(Webcam *webcam);

#endif

