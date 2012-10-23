#include "io_png.h"
#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "filters.h"
#include <unistd.h>


int main(int argc, char *argv[])
{
	/*Input*/
	size_t 	width, height, width2, height2;
	float * image1_color;
	float * image2_color;
	float * image1_gray;
	float * image2_gray;
	int 	disp_min, disp_max, disp_range;
	
	int 	optch;
	extern int opterr;
	char 	option;
	
	/*Parameter variables*/
	float 	color_threshold;
	float 	gradient_threshold;
	float 	alpha;
	int 	kernel_radius;
	float 	epsilon;
	float 	sigma_color;
	float 	sigma_spatial;
	int 	median_radius;
	int 	fill_occlusion;
	int 	detect_occlusion;
	float 	border_threshold;
	
	/*Output*/
	float * disparity;
	float * disparity_occlusion;
	float * disparity_occlusion_filled;
	
	/*Other variables*/
	int 	x, y, d, dx, dy;
	float	tmp;
	float * tmp_image1;
	float * tmp_image2;
	float * tmp_image3;
	float * tmp_image_color1;
	float * tmp_matrix1;
	float * tmp_matrix2;
	
	//images
	float * image1_r;
	float * image1_g;
	float * image1_b;
	float * image1_rr;
	float * image1_rg;
	float * image1_rb;
	float * image1_gg;
	float * image1_gb;
	float * image1_bb;

	float * gradient1;
	float * gradient2;

	float * mean_image1_r;
	float * mean_image1_g;
	float * mean_image1_b;
	float * variance_image1_rr;
	float * variance_image1_rg;
	float * variance_image1_rb;
	float * variance_image1_gg;
	float * variance_image1_gb;
	float * variance_image1_bb;
	float * mean_image1_r_disparity_cost;
	float * mean_image1_g_disparity_cost;
	float * mean_image1_b_disparity_cost;
	float * covariance_image1_r_disparity_cost;
	float * covariance_image1_g_disparity_cost;
	float * covariance_image1_b_disparity_cost;
	
	float * image1_color_filtered;
	
	//mirror images
	float * mirror_image1_color;
	float * mirror_image2_color;
	float * mirror_image1_r;
	float * mirror_image1_g;
	float * mirror_image1_b;
	float * mirror_image1_rr;
	float * mirror_image1_rg;
	float * mirror_image1_rb;
	float * mirror_image1_gg;
	float * mirror_image1_gb;
	float * mirror_image1_bb;
	float * mirror_image1_gray;
	float * mirror_image2_gray;

	float * mirror_gradient1;
	float * mirror_gradient2;

	float * mean_mirror_image1_r;
	float * mean_mirror_image1_g;
	float * mean_mirror_image1_b;
	float * variance_mirror_image1_rr;
	float * variance_mirror_image1_rg;
	float * variance_mirror_image1_rb;
	float * variance_mirror_image1_gg;
	float * variance_mirror_image1_gb;
	float * variance_mirror_image1_bb;
	float * mean_mirror_image1_r_disparity_cost;
	float * mean_mirror_image1_g_disparity_cost;
	float * mean_mirror_image1_b_disparity_cost;
	float * covariance_mirror_image1_r_disparity_cost;
	float * covariance_mirror_image1_g_disparity_cost;
	float * covariance_mirror_image1_b_disparity_cost;

	//cost-volume
	float	penalty_color, penalty_gradient, penalty;
	float * disparity_cost;
	float * mirror_disparity_cost;
	
	//guided filter
	float * mean_disparity_cost;
	float * mean_mirror_disparity_cost;
	float * filtered_cost_volume;
	float * mirror_filtered_cost_volume;
	float * sigma;
	float * mirror_disparity;
	float * inverse;
	
	/*Default parameters*/
	color_threshold = 7.0;
	gradient_threshold = 2.0;
	alpha = 1.0-0.1;
	kernel_radius = 9;
	epsilon = 0.0001*255*255;
	sigma_color = 255*0.1;
	sigma_spatial = 9.0;
	median_radius = 19;
	fill_occlusion = 0;
	detect_occlusion = 0;
	border_threshold = 3.0;

	while ((option=getopt(argc, argv, "C:G:A:R:E:B:c:s:r:oOh")) != EOF)
	{
        switch (option)
	    {
		    case 'C':
		    color_threshold = atof(optarg);
        	break;
		    case 'G':
		    gradient_threshold = atof(optarg);
        	break;
		    case 'A':
		    alpha = atof(optarg);
        	break;
		    case 'R':
		    kernel_radius = atof(optarg);
        	break;
		    case 'E':
		    epsilon = atof(optarg);
        	break;
		    case 'B':
		    border_threshold = atof(optarg);
        	break;
		    case 'c':
		    sigma_color = atof(optarg);
        	break;
		    case 's':
		    sigma_spatial = atof(optarg);
        	break;
		    case 'r':
		    median_radius = atof(optarg);
        	break;
		    case 'o':
		    detect_occlusion = 1;
        	break;
		    case 'O':
		    detect_occlusion = 1;
		    fill_occlusion = 1;
        	break;
		    case 'h':
		    printf("Usage: %s [options] -- im1.png im2.png dMin dMax\n", argv[0]);
		    printf("Options:\n");
		    printf("\t -C color_threshold\n");
		    printf("\t -G gradient_threshold\n");
		    printf("\t -A alpha\n");
		    printf("\t -R kernel_radius\n");
		    printf("\t -E epsilon\n");
		    printf("\t -B border_threshold\n");
		    printf("\t -c sigma_color\n");
		    printf("\t -s sigma_spatial\n");
		    printf("\t -r median_radius\n");
		    printf("\t -o detect occlusion\n");
		    printf("\t -O fill occlusion\n");
		    return 1;
        	break;
		}
	}

	/*Load images and convert in grayscale*/
	printf("Load images...");
	
	image1_color = read_png_f32_rgb(argv[argc-4], &width, &height);
	image2_color = read_png_f32_rgb(argv[argc-3], &width2, &height2);
	image1_gray = read_png_f32_gray(argv[argc-4], &width, &height);
	image2_gray = read_png_f32_gray(argv[argc-3], &width2, &height2);
	
	if(width != width2 || height != height2) 
	{
		printf("The images should have the same size!\n");	
		return 1;
	}

	image1_r = (float *) malloc( width * height * sizeof(float) );
	image1_g = (float *) malloc( width * height * sizeof(float) );
	image1_b = (float *) malloc( width * height * sizeof(float) );
	image1_rr = (float *) malloc( width * height * sizeof(float) );
	image1_rg = (float *) malloc( width * height * sizeof(float) );
	image1_rb = (float *) malloc( width * height * sizeof(float) );
	image1_gg = (float *) malloc( width * height * sizeof(float) );
	image1_gb = (float *) malloc( width * height * sizeof(float) );
	image1_bb = (float *) malloc( width * height * sizeof(float) );
	
	image_r(image1_color, image1_r, width, height);
	image_g(image1_color, image1_g, width, height);
	image_b(image1_color, image1_b, width, height);
	
	prod(image1_r, image1_r, image1_rr, width, height);
	prod(image1_r, image1_g, image1_rg, width, height);
	prod(image1_r, image1_b, image1_rb, width, height);
	prod(image1_g, image1_g, image1_gg, width, height);
	prod(image1_g, image1_b, image1_gb, width, height);
	prod(image1_b, image1_b, image1_bb, width, height);

	
	/*Mirror images*/
	mirror_image1_color = (float *) malloc( 3 * width * height * sizeof(float) );
	mirror_image2_color = (float *) malloc( 3 * width * height * sizeof(float) );
	mirror_image1_gray = (float *) malloc( width * height * sizeof(float) );
	mirror_image2_gray = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_r = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_g = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_b = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_rr = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_rg = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_rb = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_gg = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_gb = (float *) malloc( width * height * sizeof(float) );
	mirror_image1_bb = (float *) malloc( width * height * sizeof(float) );
	
	mirror_color(image2_color, mirror_image1_color, width, height);
	mirror_color(image1_color, mirror_image2_color, width, height);
	mirror_gray(image2_gray, mirror_image1_gray, width, height);
	mirror_gray(image1_gray, mirror_image2_gray, width, height);	
	
	image_r(mirror_image1_color, mirror_image1_r, width, height);
	image_g(mirror_image1_color, mirror_image1_g, width, height);
	image_b(mirror_image1_color, mirror_image1_b, width, height);
	
	prod(mirror_image1_r, mirror_image1_r, mirror_image1_rr, width, height);
	prod(mirror_image1_r, mirror_image1_g, mirror_image1_rg, width, height);
	prod(mirror_image1_r, mirror_image1_b, mirror_image1_rb, width, height);
	prod(mirror_image1_g, mirror_image1_g, mirror_image1_gg, width, height);
	prod(mirror_image1_g, mirror_image1_b, mirror_image1_gb, width, height);
	prod(mirror_image1_b, mirror_image1_b, mirror_image1_bb, width, height);
	
	
	printf("done.\n");
	
	
	/*Set disparity range*/
	disp_min = atoi(argv[argc-2]);
	disp_max = atoi(argv[argc-1]);
	
	if(disp_min>disp_max) 
	{
		printf("Wrong disparity range! (disp_min > disp_max)\n");
		return 1;
	}
	disp_range = disp_max - disp_min + 1;


	/*Compute gradient in X-direction from grayscale images*/
	printf("Compute gradient...");
	gradient1 = (float *) malloc( width * height * sizeof(float) );
	gradient2 = (float *) malloc( width * height * sizeof(float) );
	mirror_gradient1 = (float *) malloc( width * height * sizeof(float) );
	mirror_gradient2 = (float *) malloc( width * height * sizeof(float) );

	gradient_X(image1_gray, gradient1, width, height);
	gradient_X(image2_gray, gradient2, width, height);

	gradient_X(mirror_image1_gray, mirror_gradient1, width, height);
	gradient_X(mirror_image2_gray, mirror_gradient2, width, height);
	
	printf("done.\n");
	
	
	/*Compute the mean and variance value of each patch*/
	printf("Compute the mean and the variance values of each patch...");
	tmp_image1 = (float *) malloc( width * height * sizeof(float) );
	tmp_image2 = (float *) malloc( width * height * sizeof(float) );
	
	mean_image1_r = (float *) malloc( width * height * sizeof(float) );
	mean_image1_g = (float *) malloc( width * height * sizeof(float) );
	mean_image1_b = (float *) malloc( width * height * sizeof(float) );
	variance_image1_rr = (float *) malloc( width * height * sizeof(float) );
	variance_image1_rg = (float *) malloc( width * height * sizeof(float) );
	variance_image1_rb = (float *) malloc( width * height * sizeof(float) );
	variance_image1_gg = (float *) malloc( width * height * sizeof(float) );
	variance_image1_gb = (float *) malloc( width * height * sizeof(float) );
	variance_image1_bb = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_image1_r = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_image1_g = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_image1_b = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_rr = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_rg = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_rb = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_gg = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_gb = (float *) malloc( width * height * sizeof(float) );
	variance_mirror_image1_bb = (float *) malloc( width * height * sizeof(float) );
	
	boxfilter(image1_r, mean_image1_r, kernel_radius, width, height);
	boxfilter(image1_g, mean_image1_g, kernel_radius, width, height);
	boxfilter(image1_b, mean_image1_b, kernel_radius, width, height);
	
	boxfilter(image1_rr, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_r, mean_image1_r, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_rr, width, height);
	
	boxfilter(image1_rg, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_r, mean_image1_g, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_rg, width, height);
	
	boxfilter(image1_rb, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_r, mean_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_rb, width, height);
	
	boxfilter(image1_gg, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_g, mean_image1_g, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_gg, width, height);
	
	boxfilter(image1_gb, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_g, mean_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_gb, width, height);
	
	boxfilter(image1_bb, tmp_image1, kernel_radius, width, height);
	prod(mean_image1_b, mean_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_image1_bb, width, height);
	
	
	boxfilter(mirror_image1_r, mean_mirror_image1_r, kernel_radius, width, height);
	boxfilter(mirror_image1_g, mean_mirror_image1_g, kernel_radius, width, height);
	boxfilter(mirror_image1_b, mean_mirror_image1_b, kernel_radius, width, height);
	
	boxfilter(mirror_image1_rr, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_r, mean_mirror_image1_r, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_rr, width, height);
	
	boxfilter(mirror_image1_rg, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_r, mean_mirror_image1_g, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_rg, width, height);
	
	boxfilter(mirror_image1_rb, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_r, mean_mirror_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_rb, width, height);
	
	boxfilter(mirror_image1_gg, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_g, mean_mirror_image1_g, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_gg, width, height);
	
	boxfilter(mirror_image1_gb, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_g, mean_mirror_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_gb, width, height);
	
	boxfilter(mirror_image1_bb, tmp_image1, kernel_radius, width, height);
	prod(mean_mirror_image1_b, mean_mirror_image1_b, tmp_image2, width, height);
	substract(tmp_image1, tmp_image2, variance_mirror_image1_bb, width, height);
	
	printf("done.\n");

	/*Create initial cost volume*/
	tmp_image3 = 		(float *) malloc( width * height * sizeof(float) );
	tmp_matrix1 = 		(float *) malloc( 3 * 3 * sizeof(float) );
	tmp_matrix2 = 		(float *) malloc( 3 * 3 * sizeof(float) );
	tmp_image_color1 = 	(float *) malloc( 3 * width * height * sizeof(float) );
	if(tmp_image1 == NULL || tmp_image2 == NULL || tmp_image_color1 == NULL) return 1;

	/*
	filtered_cost_volume[x+y*width + (d-disp_min)*width*height] = filtered cost volume value for pixel (x,y) at disparity d
	*/
	disparity_cost = (float *) malloc( width * height * sizeof(float) );
	filtered_cost_volume = (float *) malloc( width * height * disp_range * sizeof(float) );
	if(disparity_cost == NULL || filtered_cost_volume == NULL) return 1;
	
	mirror_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mirror_filtered_cost_volume = (float *) malloc( width * height * disp_range * sizeof(float) );
	
	mean_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_image1_r_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_image1_g_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_image1_b_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_image1_r_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_image1_g_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_image1_b_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	
	mean_mirror_image1_r_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_image1_g_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	mean_mirror_image1_b_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_mirror_image1_r_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_mirror_image1_g_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	covariance_mirror_image1_b_disparity_cost = (float *) malloc( width * height * sizeof(float) );
	
	for(d=disp_min; d<disp_max+1; d++)
	{
		printf("disparity %d...\n", d);
		printf("\tCompute the cost-volume...");
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				penalty_color =  0;
				if(x+d<0 || x+d>width-1)
				{
					tmp = image1_color[x+y*width]
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
					
					tmp = image1_color[x+y*width + width*height]
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
					
					tmp = image1_color[x+y*width + 2*width*height]
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
				}
				else
				{
					tmp = image1_color[x+y*width]
					    - image2_color[x+d+y*width];
					if(tmp > 0)	penalty_color += tmp;
					else 		penalty_color -= tmp;
								 
					tmp = image1_color[x+y*width + width*height]
					    - image2_color[x+d+y*width + width*height];
					if(tmp > 0)	penalty_color += tmp;
					else 		penalty_color -= tmp;
								 
					tmp = image1_color[x+y*width + 2*width*height]
					    - image2_color[x+d+y*width + 2*width*height];
					if(tmp > 0) 	penalty_color += tmp;
					else 		penalty_color -= tmp;
				}
				penalty_color /= 3;
				
				if(penalty_color > color_threshold) 
				    penalty_color = color_threshold;
				
				/*SAD of gradient*/
				if(x+d < 0 || x+d>width-1)
				{
					tmp = gradient1[x+y*width]
					    - border_threshold;
					if(tmp > 0)	penalty_gradient = tmp;
					else		penalty_gradient = -tmp;
				}
				else
				{
					tmp = gradient1[x+y*width]
					    - gradient2[x+d+y*width];
					if(tmp > 0)	penalty_gradient = tmp;
					else		penalty_gradient = -tmp;
				}
				
				if(penalty_gradient > gradient_threshold) 
				    penalty_gradient = gradient_threshold;

				/*Combination of the two penalty*/
				disparity_cost[x+y*width] = (1-alpha) * penalty_color
				                          + alpha * penalty_gradient;
			
				/*Mirror*/
				/*SAD of intensity*/
				penalty_color =  0;
				if(x+d<0 || x+d>width-1)
				{
					tmp = mirror_image1_color[x+y*width]
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
					
					tmp = mirror_image1_color[x+y*width + width*height] 
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
					
					tmp = mirror_image1_color[x+y*width + 2*width*height] 
					    - border_threshold;
					if(tmp > 0)	penalty_color += tmp;
					else		penalty_color -= tmp;
				}
				else
				{
					tmp = mirror_image1_color[x+y*width] 
					    - mirror_image2_color[x+d+y*width];
					if(tmp > 0)	penalty_color += tmp;
					else 		penalty_color -= tmp;
								 
					tmp = mirror_image1_color[x+y*width + width*height] 
					    - mirror_image2_color[x+d+y*width + width*height];
					if(tmp > 0)	penalty_color += tmp;
					else 		penalty_color -= tmp;
								 
					tmp = mirror_image1_color[x+y*width + 2*width*height] 
					    - mirror_image2_color[x+d+y*width + 2*width*height];
					if(tmp > 0) 	penalty_color += tmp;
					else 		penalty_color -= tmp;
				}
				penalty_color *= 0.3333333333333333333;
				
				if(penalty_color > color_threshold) 
				    penalty_color = color_threshold;
				
				/*SAD of gradient*/
				if(x+d < 0 || x+d>width-1)
				{
					tmp = mirror_gradient1[x+y*width] 
					    - border_threshold;
					if(tmp > 0)	penalty_gradient = tmp;
					else		penalty_gradient = -tmp;
				}
				else
				{
					tmp = mirror_gradient1[x+y*width] 
					    - mirror_gradient2[x+d+y*width];
					if(tmp > 0)	penalty_gradient = tmp;
					else		penalty_gradient = -tmp;
				}
				
				if(penalty_gradient > gradient_threshold) 
				    penalty_gradient = gradient_threshold;

				/*Combination of the two penalty*/
				mirror_disparity_cost[x+y*width] = (1-alpha) * penalty_color
				                                 + alpha * penalty_gradient;
			
			}
		}

		
		/*Filter the cost-volume*/
		printf("\tFilter the cost volume..."); 
		
		//mean of disparity cost
		boxfilter(disparity_cost, mean_disparity_cost, kernel_radius, width, height);
		boxfilter(mirror_disparity_cost, mean_mirror_disparity_cost, kernel_radius, width, height);
		
		prod(image1_r, disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_image1_r_disparity_cost, kernel_radius, width, height);
		
		prod(image1_g, disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_image1_g_disparity_cost, kernel_radius, width, height);
		
		prod(image1_b, disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_image1_b_disparity_cost, kernel_radius, width, height);
		
		prod(mirror_image1_r, mirror_disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_mirror_image1_r_disparity_cost, kernel_radius, width, height);
		
		prod(mirror_image1_g, mirror_disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_mirror_image1_g_disparity_cost, kernel_radius, width, height);
		
		prod(mirror_image1_b, mirror_disparity_cost, tmp_image1, width, height);
		boxfilter(tmp_image1, mean_mirror_image1_b_disparity_cost, kernel_radius, width, height);
		
		
		prod(mean_image1_r, mean_disparity_cost, tmp_image1, width, height);
		substract(mean_image1_r_disparity_cost, tmp_image1, covariance_image1_r_disparity_cost, width, height);
		
		prod(mean_image1_g, mean_disparity_cost, tmp_image1, width, height);
		substract(mean_image1_g_disparity_cost, tmp_image1, covariance_image1_g_disparity_cost, width, height);
		
		prod(mean_image1_b, mean_disparity_cost, tmp_image1, width, height);
		substract(mean_image1_b_disparity_cost, tmp_image1, covariance_image1_b_disparity_cost, width, height);
		
		prod(mean_mirror_image1_r, mean_mirror_disparity_cost, tmp_image1, width, height);
		substract(mean_mirror_image1_r_disparity_cost, tmp_image1, covariance_mirror_image1_r_disparity_cost, width, height);
		
		prod(mean_mirror_image1_g, mean_mirror_disparity_cost, tmp_image1, width, height);
		substract(mean_mirror_image1_g_disparity_cost, tmp_image1, covariance_mirror_image1_g_disparity_cost, width, height);
		
		prod(mean_mirror_image1_b, mean_mirror_disparity_cost, tmp_image1, width, height);
		substract(mean_mirror_image1_b_disparity_cost, tmp_image1, covariance_mirror_image1_b_disparity_cost, width, height);
		
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				tmp_matrix1[0] = variance_image1_rr[x+y*width] + epsilon;
				tmp_matrix1[1] = variance_image1_rg[x+y*width];
				tmp_matrix1[2] = variance_image1_rb[x+y*width];
				tmp_matrix1[3] = variance_image1_rg[x+y*width];
				tmp_matrix1[4] = variance_image1_gg[x+y*width] + epsilon;
				tmp_matrix1[5] = variance_image1_gb[x+y*width];
				tmp_matrix1[6] = variance_image1_rb[x+y*width];
				tmp_matrix1[7] = variance_image1_gb[x+y*width];
				tmp_matrix1[8] = variance_image1_bb[x+y*width] + epsilon;
				
				inverse3(tmp_matrix1, tmp_matrix2);
				
				tmp_image_color1[x+y*width] = 
				    covariance_image1_r_disparity_cost[x+y*width] * tmp_matrix2[0]
				  + covariance_image1_g_disparity_cost[x+y*width] * tmp_matrix2[3]
				  + covariance_image1_b_disparity_cost[x+y*width] * tmp_matrix2[6];
				tmp_image_color1[x+y*width + width*height] =
				    covariance_image1_r_disparity_cost[x+y*width] * tmp_matrix2[1]
                  + covariance_image1_g_disparity_cost[x+y*width] * tmp_matrix2[4]
				  + covariance_image1_b_disparity_cost[x+y*width] * tmp_matrix2[7];
				tmp_image_color1[x+y*width + 2*width*height] = 
				    covariance_image1_r_disparity_cost[x+y*width] * tmp_matrix2[2]
				  + covariance_image1_g_disparity_cost[x+y*width] * tmp_matrix2[5]
				  + covariance_image1_b_disparity_cost[x+y*width] * tmp_matrix2[8];

				tmp_image2[x+y*width] = mean_disparity_cost[x+y*width]
				  - tmp_image_color1[x+y*width]
				    * mean_image1_r[x+y*width]
				  - tmp_image_color1[x+y*width + width*height]
				    * mean_image1_g[x+y*width]
				  - tmp_image_color1[x+y*width + 2*width*height]
				    * mean_image1_b[x+y*width];
			}
		}
		
	    
		image_r(tmp_image_color1, tmp_image1, width, height);
		boxfilter(tmp_image1, tmp_image1, kernel_radius, width, height);
		prod(tmp_image1, image1_r, tmp_image1, width, height);

		image_g(tmp_image_color1, tmp_image3, width, height);
		boxfilter(tmp_image3, tmp_image3, kernel_radius, width, height);
		prod(tmp_image3, image1_g, tmp_image3, width, height);

		sum(tmp_image1, tmp_image3, tmp_image1, width, height);

		image_b(tmp_image_color1, tmp_image3, width, height);
		boxfilter(tmp_image3, tmp_image3, kernel_radius, width, height);
		prod(tmp_image3, image1_b, tmp_image3, width, height);

		sum(tmp_image1, tmp_image3, tmp_image1, width, height);
		
		boxfilter(tmp_image2, tmp_image2, kernel_radius, width, height);
		
		sum(tmp_image2, tmp_image1, tmp_image3, width, height);
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				filtered_cost_volume[x+y*width + (d-disp_min)*width*height] =
				    tmp_image3[x+y*width];
			}
		}
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				tmp_matrix1[0] = variance_mirror_image1_rr[x+y*width] + epsilon;
				tmp_matrix1[1] = variance_mirror_image1_rg[x+y*width];
				tmp_matrix1[2] = variance_mirror_image1_rb[x+y*width];
				tmp_matrix1[3] = variance_mirror_image1_rg[x+y*width];
				tmp_matrix1[4] = variance_mirror_image1_gg[x+y*width] + epsilon;
				tmp_matrix1[5] = variance_mirror_image1_gb[x+y*width];
				tmp_matrix1[6] = variance_mirror_image1_rb[x+y*width];
				tmp_matrix1[7] = variance_mirror_image1_gb[x+y*width];
				tmp_matrix1[8] = variance_mirror_image1_bb[x+y*width] + epsilon;
				
				inverse3(tmp_matrix1, tmp_matrix2);
				
				tmp_image_color1[x+y*width] =
				    covariance_mirror_image1_r_disparity_cost[x+y*width]
				     * tmp_matrix2[0]
				  + covariance_mirror_image1_g_disparity_cost[x+y*width]
				     * tmp_matrix2[3]
				  + covariance_mirror_image1_b_disparity_cost[x+y*width]
				     * tmp_matrix2[6];
				tmp_image_color1[x+y*width + width*height] = 
				    covariance_mirror_image1_r_disparity_cost[x+y*width]
				     * tmp_matrix2[1]
				  + covariance_mirror_image1_g_disparity_cost[x+y*width]
				     * tmp_matrix2[4]
				  + covariance_mirror_image1_b_disparity_cost[x+y*width]
				     * tmp_matrix2[7];
				tmp_image_color1[x+y*width + 2*width*height] = 
				    covariance_mirror_image1_r_disparity_cost[x+y*width]
				     * tmp_matrix2[2]
				  + covariance_mirror_image1_g_disparity_cost[x+y*width]
				     * tmp_matrix2[5]
				  + covariance_mirror_image1_b_disparity_cost[x+y*width]
				     * tmp_matrix2[8];

				tmp_image2[x+y*width] = mean_mirror_disparity_cost[x+y*width]
					 - tmp_image_color1[x+y*width]
					   * mean_mirror_image1_r[x+y*width]
					 - tmp_image_color1[x+y*width + width*height]
					   * mean_mirror_image1_g[x+y*width]
					 - tmp_image_color1[x+y*width + 2*width*height]
					   * mean_mirror_image1_b[x+y*width];
			}
		}
		
		image_r(tmp_image_color1, tmp_image1, width, height);
		boxfilter(tmp_image1, tmp_image1, kernel_radius, width, height);
		prod(tmp_image1, mirror_image1_r, tmp_image1, width, height);

		image_g(tmp_image_color1, tmp_image3, width, height);
		boxfilter(tmp_image3, tmp_image3, kernel_radius, width, height);
		prod(tmp_image3, mirror_image1_g, tmp_image3, width, height);

		sum(tmp_image1, tmp_image3, tmp_image1, width, height);

		image_b(tmp_image_color1, tmp_image3, width, height);
		boxfilter(tmp_image3, tmp_image3, kernel_radius, width, height);
		prod(tmp_image3, mirror_image1_b, tmp_image3, width, height);

		sum(tmp_image1, tmp_image3, tmp_image1, width, height);
		
		boxfilter(tmp_image2, tmp_image2, kernel_radius, width, height);
		
		sum(tmp_image2, tmp_image1, tmp_image3, width, height);

		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				mirror_filtered_cost_volume[x+y*width + (d-disp_min)*width*height] = 
				    tmp_image3[x+y*width];
			}
		}
		printf("done.\n");		
	}
	
	
	free(image1_r);
	free(image1_g);
	free(image1_b);
	free(image1_rr);
	free(image1_rg);
	free(image1_rb);
	free(image1_gg);
	free(image1_gb);
	free(image1_bb);
	free(gradient1);
	free(gradient2);
	free(mean_image1_r);
	free(mean_image1_g);
	free(mean_image1_b);
	free(variance_image1_rr);
	free(variance_image1_rg);
	free(variance_image1_rb);
	free(variance_image1_gg);
	free(variance_image1_gb);
	free(variance_image1_bb);
	free(mean_image1_r_disparity_cost);
	free(mean_image1_g_disparity_cost);
	free(mean_image1_b_disparity_cost);
	free(covariance_image1_r_disparity_cost);
	free(covariance_image1_g_disparity_cost);
	free(covariance_image1_b_disparity_cost);
	free(mirror_image1_color);
	free(mirror_image2_color);
	free(mirror_image1_r);
	free(mirror_image1_g);
	free(mirror_image1_b);
	free(mirror_image1_rr);
	free(mirror_image1_rg);
	free(mirror_image1_rb);
	free(mirror_image1_gg);
	free(mirror_image1_gb);
	free(mirror_image1_bb);
	free(mirror_image1_gray);
	free(mirror_image2_gray);
	free(mirror_gradient1);
	free(mirror_gradient2);
	free(mean_mirror_image1_r);
	free(mean_mirror_image1_g);
	free(mean_mirror_image1_b);
	free(variance_mirror_image1_rr);
	free(variance_mirror_image1_rg);
	free(variance_mirror_image1_rb);
	free(variance_mirror_image1_gg);
	free(variance_mirror_image1_gb);
	free(variance_mirror_image1_bb);
	free(mean_mirror_image1_r_disparity_cost);
	free(mean_mirror_image1_g_disparity_cost);
	free(mean_mirror_image1_b_disparity_cost);
	free(covariance_mirror_image1_r_disparity_cost);
	free(covariance_mirror_image1_g_disparity_cost);
	free(covariance_mirror_image1_b_disparity_cost);
    
	free(tmp_image1);
	free(tmp_image3);
	free(tmp_image_color1);
	
	/*Winner take all label selection*/
	disparity = (float *) malloc( width * height * sizeof(float) );
	mirror_disparity = (float *) malloc( width * height * sizeof(float) );
	if(disparity == NULL || mirror_disparity == NULL) return 1;
	
	printf("Compute the disparity map...");
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			penalty = filtered_cost_volume[x+y*width];
			disparity[x+y*width] = disp_min;
			for(d=disp_min+1; d<disp_max+1; d++)
			{
				if(filtered_cost_volume[x+y*width + (d-disp_min)*width*height] < penalty
				   || filtered_cost_volume[x+y*width + (d-disp_min)*width*height] == penalty)
				{
					disparity[x+y*width] = d;
					penalty = filtered_cost_volume[x+y*width + (d-disp_min)*width*height];
				}
			}

			penalty = mirror_filtered_cost_volume[x+y*width];
			mirror_disparity[x+y*width] = disp_min; 
			for(d=disp_min+1; d<disp_max+1; d++)
			{
				if(mirror_filtered_cost_volume[x+y*width + (d-disp_min)*width*height] < penalty 
				   || mirror_filtered_cost_volume[x+y*width + (d-disp_min)*width*height] == penalty)
				{
					mirror_disparity[x+y*width] = d;
					penalty = mirror_filtered_cost_volume[x+y*width + (d-disp_min)*width*height];
				}
			}

		}
	}
	
	
	free(filtered_cost_volume);
	free(mirror_filtered_cost_volume);
	
	save_disparity("disparity.png", disparity, disp_min, disp_max, width, height);
	printf("done.\n");
	
	if(detect_occlusion == 1)
	{
		printf("Detect occlusions...");
		disparity_occlusion = (float *) malloc( width * height * sizeof(float) );
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				d = (int) disparity[x+y*width];
				if(x+d<0 || x+d>width-1) d = 0;
					
				if(mirror_disparity[width-(x+d)-1+y*width] == disparity[x+y*width])
					disparity_occlusion[x+y*width] = disparity[x+y*width];
				else
					disparity_occlusion[x+y*width] = disp_min-1;
			}
		}
		printf("done.\n");
		
	
		save_disparity_with_occlusion("disparity_occlusion.png", disparity_occlusion, 
		                              disp_min-1, disp_min, disp_max, width, height);
	}
	
	if(fill_occlusion == 1)
	{
		printf("Post-processing: Detect and fill occlusions...");
		disparity_occlusion_filled = (float *) malloc( width * height * sizeof(float) );
	    image1_color_filtered = (float *) malloc( 3 * width * height * sizeof(float) );
		
		for(x=0; x<width; x++)
		{
			for(y=0; y<height; y++)
			{
				d = (int) disparity[x+y*width];
				
				if(disparity_occlusion[x+y*width] != disp_min-1)
				{
					disparity_occlusion_filled[x+y*width] = disparity[x+y*width];
				}
				else
				{
					dx = x+1;
					while(disparity_occlusion[dx+y*width] == disp_min-1 && dx<width-1) 
					    dx++;
					
					
					if(dx<width) disparity_occlusion_filled[x+y*width] = 
					                disparity_occlusion[dx+y*width];
					else disparity_occlusion_filled[x+y*width] = disp_min;

					
					dx = x-1;
					while(disparity_occlusion[dx+y*width] == disp_min-1 && dx>0) 
					    dx--;
					
					if(dx>0)	tmp = disparity_occlusion[dx+y*width];
					else		tmp = disp_min;
					
					if(disparity_occlusion_filled[x+y*width] < tmp) 
						disparity_occlusion_filled[x+y*width] = tmp;		
				}
			}
		}
		
		save_disparity("disparity_occlusion_filled.png", disparity_occlusion_filled, 
		               disp_min, disp_max, width, height);
		printf("done.\n");

		printf("Post-processing: Smooth the disparity map...");
		median_filter(image1_color, image1_color_filtered, 0, 256, width, height, 1);
		
		weighted_median_filter(disparity_occlusion_filled, tmp_image2, image1_color_filtered,
		                       disparity_occlusion, disp_min, disp_max+1, 
		                       width, height, median_radius, sigma_spatial, sigma_color);

        free(disparity);
        free(mirror_disparity);
        free(disparity_occlusion);
		free(disparity_occlusion_filled);
		free(image1_color_filtered);
		
		printf("done.\n");
		save_disparity("disparity_occlusion_filled_smoothed.png", tmp_image2, 
		               disp_min, disp_max, width, height);
		
		
		free(tmp_image2);
			
	}

	return 0;
}
