#ifndef _IMAGE_H
#define _IMAGE_H

#include <stddef.h>

void image_r(float* image_color, float* image_red, size_t width, 
               size_t height);
void image_g(float* image_color, float* image_green, size_t width, 
               size_t height);
void image_b(float* image_color, float* image_blue, size_t width, 
               size_t height);


void mirror_color(float* image_color, float* image_mirror, size_t width, 
                    size_t height);
void mirror_gray(float* image_gray, float* image_mirror, size_t width, 
                   size_t height);


void sum(float* image_gray1, float* image_gray2, float * image_sum, 
           size_t width, size_t height);
      //image_sum = image_gray1 + image_gray2
void substract(float* image_gray1, float* image_gray2, float * image_sub, 
                 size_t width, size_t height);
      //image_sub = image_gray1 - image_gray2
void prod(float* image_gray1, float* image_gray2, float * image_prod, 
            size_t width, size_t height);
      //image_prod = image_gray1 * image_gray2


void gradient_X(float* image_gray, float* image_grad, size_t width, 
                  size_t height);


void save_disparity(char* file_name, float* disparity_map, float disp_min, 
                    float disp_max, size_t width, size_t height);
void save_disparity_with_occlusion(char* file_name, float* disparity_map, 
                                   float occlusion_value, float disp_min, 
                                   float disp_max, size_t width, size_t height);

#endif
