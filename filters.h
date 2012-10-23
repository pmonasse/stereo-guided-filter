#ifndef _FILTERS_H
#define _FILTERS_H

#include <stddef.h>

void boxfilter(float* image_gray, float* image_filtered, int filter_radius, 
                 size_t width, size_t height);
void weighted_median_filter(float* image_gray, float* imega_filtered, 
                              float* guidance_image_color, float* filtered_pixels, 
                              int value_min, int value_max, size_t width, 
                              size_t height, int median_size, float sigma_spatial, 
                              float sigma_color);
void median_filter(float* image_color, float* image_filtered, int value_min, 
                     int value_max, size_t width, size_t height, int median_size);

void inverse3(float* matrix, float* inverse);

#endif
