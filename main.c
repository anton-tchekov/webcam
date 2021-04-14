#include "webcam.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int bmp_write_rgb(const char *filename, const unsigned char *data, int width, int height);
int bmp_write_bgr(const char *filename, const unsigned char *data, int width, int height);
int bmp_write_rgba(const char *filename, const unsigned char *data, int width, int height);
int bmp_write_bgra(const char *filename, const unsigned char *data, int width, int height);
static int bmp_write_header(FILE *fp, int width, int height);
static unsigned char padding[3];

int main(void)
{
	int num = 60;
	clock_t start, end;
	double time_used;
	unsigned char *frames[60];
	Webcam webcam;
	unsigned char *buffer;
	char name[256];
	int i, devices, width, height;
	if(!(devices = webcam_init()))
	{
		fprintf(stderr, "Failed initialize webcam\n");
		return 1;
	}

	printf("Printing devices:\n");
	for(i = 0; i < devices; ++i)
	{
		printf("%d: %s\n", i, webcam_name(i, name, sizeof(name)));
	}

	width = 320;
	height = 240;

	for(i = 0; i < num; ++i)
	{
		if(!(frames[i] = malloc(4 * width * height)))
		{
			fprintf(stderr, "Failed to allocate memory for frame buffer\n");
			return 1;
		}
	}

	if(webcam_open(&webcam, 1, width, height))
	{
		fprintf(stderr, "Failed to open webcam capture\n");
		return 1;
	}

	webcam_size(&webcam, &width, &height);
	printf("Width:  %4d\n", width);
	printf("Height: %4d\n", height);

	printf("Starting Clock\n");
	start = clock();
	for(i = 0; i < num; ++i)
	{
		printf("Frame #%d\n", i);
		if(!(buffer = webcam_frame(&webcam)))
		{
			fprintf(stderr, "Failed to capture image\n");
			return 1;
		}

		memcpy(frames[i], buffer, 4 * width * height);
	}

	end = clock();
	time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("Time: %f\n", time_used);

	for(i = 0; i < num; ++i)
	{
		snprintf(name, sizeof(name), "out%2d.bmp", i);

#ifdef __WIN32
		if(bmp_write_bgra(name, frames[i], width, height))
		{
			fprintf(stderr, "Failed to write output file\n");
			return 1;
		}
#else
		if(bmp_write_rgb(name, frames[i], width, height))
		{
			fprintf(stderr, "Failed to write output file\n");
			return 1;
		}
#endif
	}

	webcam_close(&webcam);
	return 0;
}

/* -- Write Bitmap --- */
#define BYTES_PER_PIXEL   3
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

int bmp_write_rgb(const char *filename, const unsigned char *data, int width, int height)
{
	const unsigned char *p;
	int i, j, padding_size;
	FILE *fp;
	if(!(fp = fopen(filename, "wb")))
	{
		return 1;
	}

	padding_size = bmp_write_header(fp, width, height);
	for(i = height - 1; i >= 0; --i)
	{
		for(j = 0; j < width; ++j)
		{
			p = data + (3 * (i * width + j));
			fwrite(p + 2, 1, 1, fp); /* B */
			fwrite(p + 1, 1, 1, fp); /* G */
			fwrite(p + 0, 1, 1, fp); /* R */
		}

		fwrite(padding, 1, padding_size, fp);
	}

	fclose(fp);
	return 0;
}

int bmp_write_bgr(const char *filename, const unsigned char *data, int width, int height)
{
	const unsigned char *p;
	int i, j, padding_size;
	FILE *fp;
	if(!(fp = fopen(filename, "wb")))
	{
		return 1;
	}

	padding_size = bmp_write_header(fp, width, height);
	for(i = height - 1; i >= 0; --i)
	{
		for(j = 0; j < width; ++j)
		{
			p = data + (3 * (i * width + j));
			fwrite(p + 0, 1, 1, fp); /* B */
			fwrite(p + 1, 1, 1, fp); /* G */
			fwrite(p + 2, 1, 1, fp); /* R */
		}

		fwrite(padding, 1, padding_size, fp);
	}

	fclose(fp);
	return 0;
}

int bmp_write_rgba(const char *filename, const unsigned char *data, int width, int height)
{
	const unsigned char *p;
	int i, j, padding_size;
	FILE *fp;
	if(!(fp = fopen(filename, "wb")))
	{
		return 1;
	}

	padding_size = bmp_write_header(fp, width, height);
	for(i = height - 1; i >= 0; --i)
	{
		for(j = 0; j < width; ++j)
		{
			p = data + (4 * (i * width + j));
			fwrite(p + 2, 1, 1, fp); /* B */
			fwrite(p + 1, 1, 1, fp); /* G */
			fwrite(p + 0, 1, 1, fp); /* R */
		}

		fwrite(padding, 1, padding_size, fp);
	}

	fclose(fp);
	return 0;
}

int bmp_write_bgra(const char *filename, const unsigned char *data, int width, int height)
{
	const unsigned char *p;
	int i, j, padding_size;
	FILE *fp;
	if(!(fp = fopen(filename, "wb")))
	{
		return 1;
	}

	padding_size = bmp_write_header(fp, width, height);
	for(i = height - 1; i >= 0; --i)
	{
		for(j = 0; j < width; ++j)
		{
			p = data + (4 * (i * width + j));
			fwrite(p + 0, 1, 1, fp); /* B */
			fwrite(p + 1, 1, 1, fp); /* G */
			fwrite(p + 2, 1, 1, fp); /* R */
		}

		fwrite(padding, 1, padding_size, fp);
	}

	fclose(fp);
	return 0;
}

static int bmp_write_header(FILE *fp, int width, int height)
{
	unsigned char file_header[FILE_HEADER_SIZE], info_header[INFO_HEADER_SIZE];
	int i, padding_size, file_size;
	padding_size = (4 - (width * BYTES_PER_PIXEL) % 4) % 4;
	file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (BYTES_PER_PIXEL * width + padding_size) * height;
	file_header[0] = 'B';
	file_header[1] = 'M';
	file_header[2] = (unsigned char)file_size;
	file_header[3] = (unsigned char)(file_size >> 8);
	file_header[4] = (unsigned char)(file_size >> 16);
	file_header[5] = (unsigned char)(file_size >> 24);
	file_header[6] = 0;
	file_header[7] = 0;
	file_header[8] = 0;
	file_header[9] = 0;
	file_header[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);
	file_header[11] = 0;
	file_header[12] = 0;
	file_header[13] = 0;

	info_header[0] = (unsigned char)INFO_HEADER_SIZE;
	info_header[1] = 0;
	info_header[2] = 0;
	info_header[3] = 0;
	info_header[4] = (unsigned char)width;
	info_header[5] = (unsigned char)(width >> 8);
	info_header[6] = (unsigned char)(width >> 16);
	info_header[7] = (unsigned char)(width >> 24);
	info_header[8] = (unsigned char)height;
	info_header[9] = (unsigned char)(height >> 8);
	info_header[10] = (unsigned char)(height >> 16);
	info_header[11] = (unsigned char)(height >> 24);
	info_header[12] = (unsigned char)1;
	info_header[13] = 0;
	info_header[14] = (unsigned char)(BYTES_PER_PIXEL * 8);
	for(i = 15; i < INFO_HEADER_SIZE; ++i)
	{
		info_header[i] = 0;
	}

	fwrite(file_header, 1, FILE_HEADER_SIZE, fp);
	fwrite(info_header, 1, INFO_HEADER_SIZE, fp);
	return padding_size;
}

