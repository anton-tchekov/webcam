#include "webcam.h"

#ifdef __WIN32

#include <windows.h>

enum CAPTURE_PROPETIES
{
	CAPTURE_BRIGHTNESS,
	CAPTURE_CONTRAST,
	CAPTURE_HUE,
	CAPTURE_SATURATION,
	CAPTURE_SHARPNESS,
	CAPTURE_GAMMA,
	CAPTURE_COLORENABLE,
	CAPTURE_WHITEBALANCE,
	CAPTURE_BACKLIGHTCOMPENSATION,
	CAPTURE_GAIN,
	CAPTURE_PAN,
	CAPTURE_TILT,
	CAPTURE_ROLL,
	CAPTURE_ZOOM,
	CAPTURE_EXPOSURE,
	CAPTURE_IRIS,
	CAPTURE_FOCUS,
	CAPTURE_PROP_MAX
};

typedef int (*countCaptureDevicesProc)();
typedef int (*initCaptureProc)(unsigned int deviceno, CaptureParameters *aParams);
typedef void (*deinitCaptureProc)(unsigned int deviceno);
typedef void (*doCaptureProc)(unsigned int deviceno);
typedef int (*isCaptureDoneProc)(unsigned int deviceno);
typedef void (*getCaptureDeviceNameProc)(unsigned int deviceno, char *namebuffer, int bufferlength);
typedef int (*ESCAPIVersionProc)();

typedef float (*getCapturePropertyValueProc)(unsigned int deviceno, int prop);
typedef int(*getCapturePropertyAutoProc)(unsigned int deviceno, int prop);
typedef int (*setCapturePropertyProc)(unsigned int deviceno, int prop, float value, int autoval);

typedef int (*getCaptureErrorLineProc)(unsigned int deviceno);
typedef int (*getCaptureErrorCodeProc)(unsigned int deviceno);

extern countCaptureDevicesProc countCaptureDevices;
extern initCaptureProc initCapture;
extern deinitCaptureProc deinitCapture;
extern doCaptureProc doCapture;
extern isCaptureDoneProc isCaptureDone;
extern getCaptureDeviceNameProc getCaptureDeviceName;
extern ESCAPIVersionProc ESCAPIVersion;
extern getCapturePropertyValueProc getCapturePropertyValue;
extern getCapturePropertyAutoProc getCapturePropertyAuto;
extern setCapturePropertyProc setCaptureProperty;
extern getCaptureErrorLineProc getCaptureErrorLine;
extern getCaptureErrorCodeProc getCaptureErrorCode;

countCaptureDevicesProc countCaptureDevices;
initCaptureProc initCapture;
deinitCaptureProc deinitCapture;
doCaptureProc doCapture;
isCaptureDoneProc isCaptureDone;
getCaptureDeviceNameProc getCaptureDeviceName;
ESCAPIVersionProc ESCAPIVersion;
getCapturePropertyValueProc getCapturePropertyValue;
getCapturePropertyAutoProc getCapturePropertyAuto;
setCapturePropertyProc setCaptureProperty;
getCaptureErrorLineProc getCaptureErrorLine;
getCaptureErrorCodeProc getCaptureErrorCode;

typedef void (*initCOMProc)();
initCOMProc initCOM;

static int setupESCAPI(void)
{
	HMODULE capdll;
	if(!(capdll = LoadLibraryA("escapi.dll")))
	{
		return 0;
	}

	countCaptureDevices = (countCaptureDevicesProc)GetProcAddress(capdll, "countCaptureDevices");
	initCapture = (initCaptureProc)GetProcAddress(capdll, "initCapture");
	deinitCapture = (deinitCaptureProc)GetProcAddress(capdll, "deinitCapture");
	doCapture = (doCaptureProc)GetProcAddress(capdll, "doCapture");
	isCaptureDone = (isCaptureDoneProc)GetProcAddress(capdll, "isCaptureDone");
	initCOM = (initCOMProc)GetProcAddress(capdll, "initCOM");
	getCaptureDeviceName = (getCaptureDeviceNameProc)GetProcAddress(capdll, "getCaptureDeviceName");
	ESCAPIVersion = (ESCAPIVersionProc)GetProcAddress(capdll, "ESCAPIVersion");
	getCapturePropertyValue = (getCapturePropertyValueProc)GetProcAddress(capdll, "getCapturePropertyValue");
	getCapturePropertyAuto = (getCapturePropertyAutoProc)GetProcAddress(capdll, "getCapturePropertyAuto");
	setCaptureProperty = (setCapturePropertyProc)GetProcAddress(capdll, "setCaptureProperty");
	getCaptureErrorLine = (getCaptureErrorLineProc)GetProcAddress(capdll, "getCaptureErrorLine");
	getCaptureErrorCode = (getCaptureErrorCodeProc)GetProcAddress(capdll, "getCaptureErrorCode");

	if(!initCOM ||
		!ESCAPIVersion ||
		!getCaptureDeviceName ||
		!countCaptureDevices ||
		!initCapture ||
		!deinitCapture ||
		!doCapture ||
		!isCaptureDone ||
		!getCapturePropertyValue ||
		!getCapturePropertyAuto ||
		!setCaptureProperty ||
		!getCaptureErrorLine ||
		!getCaptureErrorCode)
	{
		return 0;
	}

	if(ESCAPIVersion() < 0x300)
	{
		return 0;
	}

	initCOM();
	return countCaptureDevices();
}

int webcam_init(void)
{
	return setupESCAPI();
}

int webcam_open(Webcam *webcam, int device_id, int width, int height)
{
	webcam->DeviceID = device_id;
	webcam->Capture.Width = width;
	webcam->Capture.Height = height;
	if(!(webcam->Capture.Buffer = malloc(width * height * sizeof(*webcam->Capture.Buffer))))
	{
		return 1;
	}

	if(initCapture(device_id, &webcam->Capture) == 0)
	{
		free(webcam->Capture.Buffer);
		return 1;
	}

	return 0;
}

void webcam_size(Webcam *webcam, int *width, int *height)
{
	*width = webcam->Capture.Width;
	*height = webcam->Capture.Height;
}

char *webcam_name(int device_id, char *name, int len)
{
	getCaptureDeviceName(device_id, name, len);
	return name;
}

unsigned char *webcam_frame(Webcam *webcam)
{
	doCapture(webcam->DeviceID);
	while(!isCaptureDone(webcam->DeviceID)) ;
	return (unsigned char *)webcam->Capture.Buffer;
}

void webcam_close(Webcam *webcam)
{
	deinitCapture(webcam->DeviceID);
}

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
#include <libv4l2.h>
#include <time.h>

static char *dev_name = "/dev/video0";

static int xioctl(int fh, unsigned long request, void *arg)
{
	int r;
	do
	{
		r = v4l2_ioctl(fh, request, arg);
	} while(r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	if(r == -1)
	{
		return 1;
	}

	return 0;
}

int webcam_init(void)
{
	return 1;
}

int webcam_open(Webcam *webcam, int device_id, int width, int height)
{
	enum v4l2_buf_type type;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	unsigned int i;

	webcam->fd = -1;
	if((webcam->fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0)) < 0)
	{
		return 1;
	}

	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(xioctl(webcam->fd, VIDIOC_S_FMT, &fmt))
	{
		return 1;
	}

	if(fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24)
	{
		return 1;
	}

	webcam->width = fmt.fmt.pix.width;
	webcam->height = fmt.fmt.pix.height;

	memset(&req, 0, sizeof(req));
	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if(xioctl(webcam->fd, VIDIOC_REQBUFS, &req))
	{
		return 1;
	}

	webcam->buffers = calloc(req.count, sizeof(*webcam->buffers));
	for(webcam->n_buffers = 0; webcam->n_buffers < req.count; ++webcam->n_buffers)
	{
		memset(&webcam->buf, 0, sizeof(webcam->buf));
		webcam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		webcam->buf.memory = V4L2_MEMORY_MMAP;
		webcam->buf.index = webcam->n_buffers;
		if(xioctl(webcam->fd, VIDIOC_QUERYBUF, &webcam->buf))
		{
			return 1;
		}

		webcam->buffers[webcam->n_buffers].length = webcam->buf.length;
		webcam->buffers[webcam->n_buffers].start = v4l2_mmap(NULL, webcam->buf.length,
				PROT_READ | PROT_WRITE, MAP_SHARED, webcam->fd, webcam->buf.m.offset);

		if(webcam->buffers[webcam->n_buffers].start == MAP_FAILED)
		{
			return 1;
		}
	}

	for(i = 0; i < webcam->n_buffers; ++i)
	{
		memset(&webcam->buf, 0, sizeof(webcam->buf));
		webcam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		webcam->buf.memory = V4L2_MEMORY_MMAP;
		webcam->buf.index = i;
		if(xioctl(webcam->fd, VIDIOC_QBUF, &webcam->buf))
		{
			return 1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(xioctl(webcam->fd, VIDIOC_STREAMON, &type))
	{
		return 1;
	}

	return 0;
	(void)device_id;
}

void webcam_size(Webcam *webcam, int *width, int *height)
{
	*width = webcam->width;
	*height = webcam->height;
}

/* Not implemented for linux */
char *webcam_name(int device_id, char *name, int len)
{
	return NULL;
	(void)device_id;
	(void)name;
	(void)len;
}

unsigned char *webcam_frame(Webcam *webcam)
{
	fd_set fds;
	int r;
	struct timeval tv;
	do
	{
		FD_ZERO(&fds);
		FD_SET(webcam->fd, &fds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select(webcam->fd + 1, &fds, NULL, NULL, &tv);
	} while((r == -1 && (errno = EINTR)));
	if(r == -1)
	{
		return NULL;
	}

	memset(&webcam->buf, 0, sizeof(webcam->buf));
	webcam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	webcam->buf.memory = V4L2_MEMORY_MMAP;
	if(xioctl(webcam->fd, VIDIOC_DQBUF, &webcam->buf))
	{
		return NULL;
	}

	if(xioctl(webcam->fd, VIDIOC_QBUF, &webcam->buf))
	{
		return NULL;
	}

	return webcam->buffers[webcam->buf.index].start;
}

void webcam_close(Webcam *webcam)
{
	unsigned int i;
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(webcam->fd, VIDIOC_STREAMOFF, &type);
	for(i = 0; i < webcam->n_buffers; ++i)
	{
		v4l2_munmap(webcam->buffers[i].start, webcam->buffers[i].length);
	}

	v4l2_close(webcam->fd);
}

#endif

