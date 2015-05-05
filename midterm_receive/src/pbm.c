//
//  pbm.c
//  pbm_rw
//
//  Created by Artem Lenskiy on 4/16/15.
//  Copyright (c) 2015 Artem Lenskiy. All rights reserved.
//

#include "pbm.h"

char *readImage(const char *file_name, unsigned long *length,
		struct image_header *ih) {
	FILE *pgmFile;
	int i, j;
	int pixel;

	char *image_data;
	char *pix;

	pgmFile = fopen(file_name, "rb");
	if (pgmFile == NULL) {
		fprintf(stderr, "cannot open file to read");
		fclose(pgmFile);
		return NULL;
	}

	fgets(ih->format, sizeof(ih->format), pgmFile);
	if (strcmp(ih->format, "P5") != 0 && strcmp(ih->format, "P6") != 0) {
		fprintf(stderr, "Wrong file type!\n");
		fclose(pgmFile);
		return NULL;
	}
	//read header
	fscanf(pgmFile, "%d", &ih->cols);
	fscanf(pgmFile, "%d", &ih->rows);
	fscanf(pgmFile, "%d", &ih->levels);
	fgetc(pgmFile);

	if (strcmp(ih->format, "P6") == 0) {
		image_data = (unsigned char *) malloc(3 * ih->rows * ih->cols);
	} else {
		printf("Wrong img file format\n");
		exit(0);
	}


	for (j = 0; j < ih->rows; ++j)
		for (i = 0; i < ih->cols; ++i) {
			pix = &image_data[j * (ih->cols * 3) + i * 3];
			pix[0] = (char) fgetc(pgmFile);
			pix[1] = (char) fgetc(pgmFile);
			pix[2] = (char) fgetc(pgmFile);
		}


	*length = 3 * ih->rows * ih->cols;
	printf("[sender]RGB Origin size: [%d]\n",*length);

	fclose(pgmFile);
	return image_data;
}

void writeImage(const char *filename, const char *image_data,
		const struct image_header ih) {
	FILE *pgmFile;
	int i, j;
	const char *pix;

	pgmFile = fopen(filename, "wb");
	if (pgmFile == NULL) {
		perror("cannot open file to write");
		exit(EXIT_FAILURE);
	}

	fprintf(pgmFile, "%s ", ih.format);
	fprintf(pgmFile, "%d %d ", ih.cols, ih.rows);
	fprintf(pgmFile, "%d ", ih.levels);

	for (j = 0; j < ih.rows; ++j){
			for (i = 0; i < ih.cols; ++i) {
				pix = &image_data[j * ih.cols * 3 + i * 3];
				fputc(pix[0], pgmFile);
				fputc(pix[1], pgmFile);
				fputc(pix[2], pgmFile);
			}
	}
	fclose(pgmFile);
}

//Step1. RGB -> YCbCr, No Make File
unsigned char *getRGBtoYCbCr(const char *filename, const unsigned char *rgb_data, const struct image_header ih) {
//	FILE *pgmFile;
	int i;
	unsigned long length;
	unsigned char *yuv_data;

	length = ih.cols * ih.rows * 3;
	yuv_data = (unsigned char*) malloc(length);

//	pgmFile = fopen(filename, "wb");
//	if (pgmFile == NULL) {
//		perror("cannot open file to write");
//		exit(EXIT_FAILURE);
//	}
//	fprintf(pgmFile, "%s ", ih.format);
//	fprintf(pgmFile, "%d %d ", ih.cols, ih.rows);
//	fprintf(pgmFile, "%d ", ih.levels);

	if (strcmp(ih.format, "P6") == 0) {
		for (i = 0; i < length; i += 3) {
			yuv_data[i] = ((19595 * rgb_data[i]) + (38470 * rgb_data[i + 1])+ (7471 * rgb_data[i + 2])) >> 16;
			yuv_data[i + 1] = (36962 * (rgb_data[i + 2] - yuv_data[i]) >> 16)+ 128;
			yuv_data[i + 2] = (46727 * (rgb_data[i] - yuv_data[i]) >> 16) + 128;
		}
//		for (i = 0; i < length; i++)
//			fputc(yuv_data[i], pgmFile);
	}

	return yuv_data;
}

//Step2. YCbCr -> YUV420
unsigned char *getYCbCrtoYUV420(const unsigned char *ycbcr_data, const struct image_header ih) {
	int width, height, i, j,k = 0;
	unsigned char *yuv420;
	width = ih.cols;
	height = ih.rows;
	yuv420 = (unsigned char*) malloc((width * height * 3) / 2);

	for (j = 0; j < width * height * 3; j += width * 6) {
		for (i = j; i < (width * 3) + j; i += 6) {

			yuv420[k++] = ycbcr_data[i];
			yuv420[k++] = ycbcr_data[i+3];
			yuv420[k++] = ycbcr_data[i+(width*3)];
			yuv420[k++] = ycbcr_data[i+(width*3)+3];

			yuv420[k++] = ycbcr_data[(i+1)] ;
			yuv420[k++] = ycbcr_data[(i+2)] ;
		}
	}

	printf("[sender]yuv420 compress size: [%d]\n",k);
	return yuv420;
}

//Step3. YUV420 -> YCbCr
unsigned char *getYUV420toYCbCr(const unsigned char *yuv420, const struct image_header ih) {
//	FILE *pgmFile;
	int i, r = 1, width, height, k = 0;
	unsigned char *ycbcr;
	width = ih.cols;
	height = ih.rows;

	//Make file!
//	pgmFile = fopen("received_con_ycbcr.pbm", "wb");
//	fprintf(pgmFile, "%s ", ih.format);
//	fprintf(pgmFile, "%d %d ", ih.cols, ih.rows);
//	fprintf(pgmFile, "%d ", ih.levels);

	ycbcr = (unsigned char*) malloc((width * height * 3));

	for (i = 0; i < (width * height * 3) / 2; i += 6) {
		ycbcr[k] = yuv420[i];
		ycbcr[k + 1] = yuv420[i + 4];
		ycbcr[k + 2] = yuv420[i + 5];

		ycbcr[k + 3] = yuv420[i + 1];
		ycbcr[k + 4] = yuv420[i + 4];
		ycbcr[k + 5] = yuv420[i + 5];

		ycbcr[k + (width * 3)] = yuv420[i + 2];
		ycbcr[(k + 1) + (width * 3)] = yuv420[i + 4];
		ycbcr[(k + 2) + (width * 3)] = yuv420[i + 5];

		ycbcr[(k + 3) + (width * 3)] = yuv420[i + 3];
		ycbcr[(k + 4) + (width * 3)] = yuv420[i + 4];
		ycbcr[(k + 5) + (width * 3)] = yuv420[i + 5];

		if ((width * 3 * r) <= k + 6) {
			r += 2;
			k += 6 + (width * 3);
		} else	k += 6;
	}

//	for(i=0;i<width*height*3;i+=3){
//		fputc(ycbcr[i], pgmFile);
//		fputc(ycbcr[i+1], pgmFile);
//		fputc(ycbcr[i+2], pgmFile);
//	}
//	fclose(pgmFile);

	return ycbcr;
}

//Step4. YCbCr -> RGB, Make File
void *getYCbCrtoRGB(const char *filename, const unsigned char *ycbcr, const struct image_header ih) {
	FILE *pgmFile;
	int i, width, height;
	unsigned char tmp1, tmp2, tmp3;
	width = ih.cols;
	height = ih.rows;

	pgmFile = fopen(filename, "wb");
	if (pgmFile == NULL) {
		perror("cannot open file to write");
		exit(EXIT_FAILURE);
	}

	fprintf(pgmFile, "%s ", ih.format);
	fprintf(pgmFile, "%d %d ", width, height);
	fprintf(pgmFile, "%d ", ih.levels);

	for (i = 0; i < width * height * 3; i += 3) {

		tmp1 = ycbcr[i] + ((91881 * ycbcr[i + 2]) >> 16) - 179;
		tmp2 = ycbcr[i] - (((46793 * ycbcr[i + 2]) + (22544 * ycbcr[i + 1])) >> 16)+ 135;
		tmp3 = ycbcr[i] + ((116129 * ycbcr[i + 1]) >> 16) - 226;

		fputc(tmp1, pgmFile);
		fputc(tmp2, pgmFile);
		fputc(tmp3, pgmFile);
	}

	fclose(pgmFile);
}
