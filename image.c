#include "image.h"
#include <stdlib.h>

void image_r(float* image_color, float* image_red, size_t width, size_t height)
{
	int x, y;
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_red[x+y*width] = image_color[x+y*width];
		}
	}
}


void image_g(float* image_color, float* image_green, size_t width, size_t height)
{
	int x, y;
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_green[x+y*width] = image_color[x+y*width + width*height];
		}
	}
}


void image_b(float* image_color, float* image_blue, size_t width, size_t height)
{
	int x, y;
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_blue[x+y*width] = image_color[x+y*width + 2*width*height];
		}
	}
}


void mirror_color(float* image_color, float* image_mirror, size_t width, size_t height)
{
	int x, y;
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_mirror[x+y*width] = 
			                image_color[width-x-1+y*width];
			image_mirror[x+y*width + width*height] = 
			                image_color[width-x-1+y*width + width*height];
			image_mirror[x+y*width + 2*width*height] = 
			                image_color[width-x-1+y*width + 2*width*height];
		}
	}
}


void mirror_gray(float* image_gray, float* image_mirror, size_t width, size_t height)
{
	int x, y;
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_mirror[x+y*width] = image_gray[width-x-1+y*width];
		}
	}
}


void sum(float* image_gray1, float* image_gray2, float* image_sum, size_t width, size_t height)
{
	int x, y;

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_sum[x+y*width] = image_gray1[x+y*width] + image_gray2[x+y*width];
		}
	}
}


void substract(float* image_gray1, float* image_gray2, float* image_sub, size_t width, size_t height)
{
	int x, y;

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_sub[x+y*width] = image_gray1[x+y*width] - image_gray2[x+y*width];
		}
	}
}


void prod(float* image_gray1, float* image_gray2, float* image_prod, size_t width, size_t height)
{
	int x, y;

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			image_prod[x+y*width] = image_gray1[x+y*width] * image_gray2[x+y*width];
		}
	}
}


void gradient_X(float* image_gray, float* image_grad, size_t width, size_t height)
{
	int x, y;

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			if(x == 0 || x == width-1) image_grad[x+y*width] = 0;
			else image_grad[x+y*width] = image_gray[x-1+y*width]
			                           + image_gray[x+1+y*width]
			                           - 2*image_gray[x-1+y*width];
			image_grad[x+y*width] += 128;
		}
	}
}


void save_disparity(char* file_name, float* disparity_map, float disp_min, float disp_max, size_t width, size_t height)
{
	float * tmp_image;
	float	tmp;
	int 	x, y;
	
	tmp_image = (float *) malloc( width * height * sizeof(float) );
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			tmp = disp_max;
			tmp -= disparity_map[x+y*width];
			tmp_image[x+y*width] = 255*tmp/(disp_max-disp_min);
		}
	}
	write_png_f32(file_name, tmp_image, width, height, 1);
	free(tmp_image);
}


void save_disparity_with_occlusion(char* file_name, float* disparity_map, float occlusion_value, float disp_min, float disp_max, size_t width, size_t height)
{
	float * tmp_image;
	float	tmp;
	int 	x, y;
	
	tmp_image = (float *) malloc( 3 * width * height * sizeof(float) );
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			tmp = disparity_map[x+y*width];
			if(tmp != occlusion_value)
			{
				tmp = disp_max;
				tmp -= disparity_map[x+y*width];
				tmp_image[x+y*width] = 255*tmp/(disp_max-disp_min);
				tmp_image[x+y*width + width*height] = 255*tmp/(disp_max-disp_min);
				tmp_image[x+y*width + 2*width*height] = 255*tmp/(disp_max-disp_min);
			}
			else
			{
				tmp_image[x+y*width] = 255;
				tmp_image[x+y*width + width*height] = 0;
				tmp_image[x+y*width + 2*width*height] = 0;
			}
				
		}
	}
	write_png_f32(file_name, tmp_image, width, height, 3);
	free(tmp_image);
}

