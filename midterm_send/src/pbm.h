//
//  pbm.h
//  pbm_rw
//
//  Created by Artem Lenskiy on 4/16/15.
//  Copyright (c) 2015 Artem Lenskiy. All rights reserved.
//

#ifndef pbm_rw_pbm_h
#define pbm_rw_pbm_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h> //for memcpy
#include <math.h>

struct image_header{
    char format[3]; //Image format, example: P5
    int rows;       //Image height
    int cols;       //Image width
    int levels;     //Number of gray/each color levels
};

char *readImage(const char *file_name, unsigned long *length, struct image_header *ih);
void writeImage(const char *filename, const char *image_data, const struct image_header ih);

unsigned char *getRGBtoYCbCr(const char *filename ,const unsigned char *rgb_data, const struct image_header ih);
unsigned char *getYCbCrtoYUV420(const unsigned char *ycbcr_data, const struct image_header ih);

unsigned char *getYUV420toYCbCr(const unsigned char *ycbcr_data, const struct image_header ih);
void *getYCbCrtoRGB(const char *filename, const unsigned char *ycbcr_data, const struct image_header ih);

#endif
