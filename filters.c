#include "filters.h"
#include <stdlib.h>
#include <math.h>

void boxfilter(float* image_gray, float* image_filtered, int filter_radius, size_t width, size_t height)
{
	float * tmp_image;
	int x, y, xmin, xmax, ymin, ymax;
	
	tmp_image = (float *) malloc( width * height * sizeof(float) );
	
	//cumulative sum table
	for(x=0; x<width; x++)
	{
		image_filtered[x] = image_gray[x];
	}
	for(y=1; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			image_filtered[x+y*width] = image_filtered[x+(y-1)*width] + image_gray[x+y*width];
		}
	}
	
	for(y=0; y<height; y++)
	{
		tmp_image[y*width] = image_filtered[y*width];
	}
	for(x=1; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			tmp_image[x+y*width] = tmp_image[x-1+y*width] + image_filtered[x+y*width];
		}
	}
	
	//box filter
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			xmin = -1;
			xmax = width-1;
			ymin = -1;
			ymax = height-1;
			
			if(x-filter_radius-1>xmin) 	xmin = x-filter_radius-1;
			if(x+filter_radius<xmax) 	xmax = x+filter_radius;
			if(y-filter_radius-1>ymin) 	ymin = y-filter_radius-1;
			if(y+filter_radius<ymax) 	ymax = y+filter_radius;
			
			image_filtered[x+y*width] = tmp_image[xmax + ymax*width];
			
			if(xmin>-1)
			{
				image_filtered[x+y*width] -= tmp_image[xmin + ymax*width];
				if(ymin>-1)
				{
					image_filtered[x+y*width] -= tmp_image[xmax + ymin*width];
					image_filtered[x+y*width] += tmp_image[xmin + ymin*width];
				}
			}
			else
			{
				if(ymin>-1)
				{
					image_filtered[x+y*width] -= tmp_image[xmax + ymin*width];
				}
			}
			
			image_filtered[x+y*width] /= (xmax-xmin)*(ymax-ymin);			
		}
	}
	free(tmp_image);
}

void weighted_median_filter(float* image_gray, float* image_filtered, float* guidance_image_color, float* filtered_pixels, int value_min, int value_max, size_t width, size_t height, int median_size, float sigma_spatial, float sigma_color)
{	
	float * tmp_tab;
	int	x, y, dx, dy, d;
	float	tmp1, tmp2;
	
	tmp_tab = (float *) malloc( (value_max-value_min) * sizeof(float) );
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			if(filtered_pixels[x+y*width] == value_min-1)
			{
				for(d=value_min; d<value_max; d++)
				{
					tmp_tab[d-value_min] = 0;
				}
				
				tmp1 = 0;	
				for(dx=-median_size; dx<median_size+1; dx++)
				{
					for(dy=-median_size; dy<median_size+1; dy++)
					{
						if(x+dx>-1 && x+dx<width && y+dy>-1 && y+dy<height)
						{
							tmp2 = exp(-(dx*dx+dy*dy)/sigma_spatial/sigma_spatial)
							     * exp(-((guidance_image_color[x+y*width]
							             -guidance_image_color[(x+dx)+(y+dy)*width]) 
							 	 * (guidance_image_color[x+y*width]
							 	   -guidance_image_color[(x+dx)+(y+dy)*width]) 
							 	 + (guidance_image_color[x+y*width + width*height]
							 	   -guidance_image_color[(x+dx)+(y+dy)*width + width*height]) 
							 	 * (guidance_image_color[x+y*width + width*height]
							 	   -guidance_image_color[(x+dx)+(y+dy)*width + width*height]) 
							 	 + (guidance_image_color[x+y*width + 2*width*height]
							 	   -guidance_image_color[(x+dx)+(y+dy)*width + 2*width*height]) 
							 	 * (guidance_image_color[x+y*width + 2*width*height]
							 	   -guidance_image_color[(x+dx)+(y+dy)*width + 2*width*height]))
							     /sigma_color/sigma_color);
							
							d = (int) image_gray[x+dx+(y+dy)*width];
								
							tmp_tab[d-value_min] += tmp2;
							tmp1 += tmp2;
						}
					}
				}
					
				d = value_min;
				tmp2 = tmp_tab[d-value_min];
				while(tmp2<tmp1/2 && d<value_max)
				{
					d++;
					tmp2 += tmp_tab[d-value_min];
				}
				image_filtered[x+y*width] = d;
			}
			else image_filtered[x+y*width] = image_gray[x+y*width];
		}
	}
	free(tmp_tab);
}


void median_filter(float* image_color, float* image_filtered, int value_min, int value_max, size_t width, size_t height, int median_size)
{	
	float * tmp_tab;
	int	x, y, dx, dy, d;
	float	tmp1, tmp2;
	
	tmp_tab = (float *) malloc( 3 * (value_max-value_min) * sizeof(float) );
	
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			for(d=value_min; d<value_max; d++)
			{
				tmp_tab[d-value_min] = 0;
				tmp_tab[d-value_min + value_max-value_min] = 0;
				tmp_tab[d-value_min + 2*(value_max-value_min)] = 0;
			}
			
			tmp1 = 0;
				
			for(dx=-median_size; dx<median_size+1; dx++)
			{
				for(dy=-median_size; dy<median_size+1; dy++)
				{
					if(x+dx>-1 && x+dx<width && y+dy>-1 && y+dy<height)
					{
						tmp2 = 1;
						d = (int) image_color[x+dx+(y+dy)*width];
						tmp_tab[d-value_min] += tmp2;
						
						d = (int) image_color[x+dx+(y+dy)*width + width*height];
						tmp_tab[d-value_min + value_max-value_min] += tmp2;
						
						d = (int) image_color[x+dx+(y+dy)*width + 2*width*height];
						tmp_tab[d-value_min + 2*(value_max-value_min)] += tmp2;
						tmp1 += tmp2;
					}
				}
			}
				
			d = value_min;
			tmp2 = tmp_tab[d-value_min];
			while(tmp2<tmp1/2 && d<value_max)
			{
				d++;
				tmp2 += tmp_tab[d-value_min];
			}
			image_filtered[x+y*width] = d;

			d = value_min;
			tmp2 = tmp_tab[d-value_min + value_max-value_min];
			while(tmp2<tmp1/2 && d<value_max)
			{
				d++;
				tmp2 += tmp_tab[d-value_min + value_max-value_min];
			}
			image_filtered[x+y*width + width*height] = d;

			d = value_min;
			tmp2 = tmp_tab[d-value_min + 2*(value_max-value_min)];
			while(tmp2<tmp1/2 && d<value_max)
			{
				d++;
				tmp2 += tmp_tab[d-value_min + 2*(value_max-value_min)];
			}
			image_filtered[x+y*width + 2*width*height] = d;

		}
	}
	free(tmp_tab);
}

void inverse3(float* matrix, float* inverse)
{
	float	det;

	det = matrix[0] * (matrix[4]*matrix[8] - matrix[5]*matrix[7])
		 - matrix[3] * (matrix[1]*matrix[8] - matrix[2]*matrix[7]) 
		 + matrix[6] * (matrix[1]*matrix[5] - matrix[2]*matrix[4]);
	
	if(det == 0) inverse = NULL;
	
	inverse[0] = (matrix[4]*matrix[8] - matrix[5]*matrix[7]) / det;
	inverse[1] = (matrix[2]*matrix[7] - matrix[1]*matrix[8]) / det;
	inverse[2] = (matrix[1]*matrix[5] - matrix[2]*matrix[4]) / det;
	inverse[3] = (matrix[5]*matrix[6] - matrix[3]*matrix[8]) / det;
	inverse[4] = (matrix[0]*matrix[8] - matrix[2]*matrix[6]) / det;
	inverse[5] = (matrix[2]*matrix[3] - matrix[0]*matrix[5]) / det;
	inverse[6] = (matrix[3]*matrix[7] - matrix[4]*matrix[6]) / det;
	inverse[7] = (matrix[1]*matrix[6] - matrix[0]*matrix[7]) / det;
	inverse[8] = (matrix[0]*matrix[4] - matrix[1]*matrix[3]) / det;
	
}

