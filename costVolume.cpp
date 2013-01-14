#include "costVolume.h"
#include "image.h"
#include "io_png.h"
#include <algorithm>
#include <limits>
#include <iostream>

/// Inverse of 3x3 matrix
static void inverse3(const float* matrix, float* inverse) {
    float det = matrix[0] * (matrix[4]*matrix[8] - matrix[5]*matrix[7])
        - matrix[3] * (matrix[1]*matrix[8] - matrix[2]*matrix[7]) 
        + matrix[6] * (matrix[1]*matrix[5] - matrix[2]*matrix[4]);
    det = 1/det;
    inverse[0] = (matrix[4]*matrix[8] - matrix[5]*matrix[7]) * det;
    inverse[1] = (matrix[2]*matrix[7] - matrix[1]*matrix[8]) * det;
    inverse[2] = (matrix[1]*matrix[5] - matrix[2]*matrix[4]) * det;
    inverse[3] = (matrix[5]*matrix[6] - matrix[3]*matrix[8]) * det;
    inverse[4] = (matrix[0]*matrix[8] - matrix[2]*matrix[6]) * det;
    inverse[5] = (matrix[2]*matrix[3] - matrix[0]*matrix[5]) * det;
    inverse[6] = (matrix[3]*matrix[7] - matrix[4]*matrix[6]) * det;
    inverse[7] = (matrix[1]*matrix[6] - matrix[0]*matrix[7]) * det;
    inverse[8] = (matrix[0]*matrix[4] - matrix[1]*matrix[3]) * det;
}

/// Compute image of matching costs at disparity \a d.
///
/// At each pixel, a linear combination of colors L1 distance (with max
/// threshold) and x-derivatives absolute difference (with max threshold).
static void compute_cost(Image im1R, Image im1G, Image im1B,
                         Image im2R, Image im2G, Image im2B,
                         Image gradient1, Image gradient2,
                         int d, const ParamCVFilter& param,
                         Image& disparity_cost) {
    const int width=im1R.width(), height=im1R.height();
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
            // Color L1 distance
            float penalty_color = 0;
            float col1[3] = {im1R(x,y), im1G(x,y), im1B(x,y)};
            float col2[3] = {param.border_threshold,
                             param.border_threshold,
                             param.border_threshold};
            if(0<=x+d && x+d<width) {
                col2[0] = im2R(x+d,y);
                col2[1] = im2G(x+d,y);
                col2[2] = im2B(x+d,y);
            }
            for(int i=0; i<3; i++) {
                float tmp = col1[i]-col2[i];
                if(tmp<0) tmp=-tmp;
                penalty_color += tmp;
            }
            penalty_color /= 3;

            if(penalty_color > param.color_threshold) 
                penalty_color = param.color_threshold;

            // x-derivative absolute difference
            float g2 = (0<=x+d && x+d<width)?
                gradient2(x+d,y): param.border_threshold;
            float penalty_gradient = gradient1(x,y)-g2;
            if(penalty_gradient < 0)
                penalty_gradient = -penalty_gradient;
            if(penalty_gradient > param.gradient_threshold) 
                penalty_gradient = param.gradient_threshold;

            // Combination of the two penalties
            disparity_cost(x,y) = (1-param.alpha) * penalty_color +
                                     param.alpha  * penalty_gradient;
        }
}


/// Cost volume filtering
Image filter_cost_volume(Image im1Color, Image im2Color,
                         int dispMin, int dispMax,
                         const ParamCVFilter& param) {
    Image im1R=im1Color.r(), im1G=im1Color.g(), im1B=im1Color.b();
    Image im2R=im2Color.r(), im2G=im2Color.g(), im2B=im2Color.b();
    const int width=im1R.width(), height=im1R.height();
    std::cout << "Cost-volume: " << (dispMax-dispMin+1) << " disparities. ";

    Image disparity(width,height);
    std::fill_n(&disparity(0,0), width*height, dispMin-1);
    Image cost(width,height);
    std::fill_n(&cost(0,0), width*height, std::numeric_limits<float>::max());

    Image im1Gray(width,height);
    Image im2Gray(width,height);
    rgb_to_gray(&im1R(0,0),&im1G(0,0),&im1B(0,0), width,height, &im1Gray(0,0));
    rgb_to_gray(&im2R(0,0),&im2G(0,0),&im2B(0,0), width,height, &im2Gray(0,0));
    Image gradient1 = im1Gray.gradX();
    Image gradient2 = im2Gray.gradX();

    // Compute the mean and variance of each patch
    Image meanIm1R = im1R.boxFilter(param.kernel_radius);
    Image meanIm1G = im1G.boxFilter(param.kernel_radius);
    Image meanIm1B = im1B.boxFilter(param.kernel_radius);

    Image varIm1RR =
        (im1R * im1R).boxFilter(param.kernel_radius) - meanIm1R*meanIm1R;
    Image varIm1RG =
        (im1R * im1G).boxFilter(param.kernel_radius) - meanIm1R*meanIm1G;
    Image varIm1RB =
        (im1R * im1B).boxFilter(param.kernel_radius) - meanIm1R*meanIm1B;
    Image varIm1GG =
        (im1G * im1G).boxFilter(param.kernel_radius) - meanIm1G*meanIm1G;
    Image varIm1GB =
        (im1G * im1B).boxFilter(param.kernel_radius) - meanIm1G*meanIm1B;
    Image varIm1BB =
        (im1B * im1B).boxFilter(param.kernel_radius) - meanIm1B*meanIm1B;

    Image tmpIm1R(width,height),tmpIm1G(width,height),tmpIm1B(width,height);
    Image disparity_cost(width,height);
    for(int d=dispMin; d<dispMax+1; d++) {
        std::cout << '*' << std::flush;
        compute_cost(im1R,im1G,im1B, im2R,im2G,im2B, gradient1, gradient2,
                     d, param, disparity_cost);
        //mean of disparity cost
        Image mean_disparity_cost = disparity_cost.boxFilter(param.kernel_radius);
        Image mean_image1_r_disparity_cost = (im1R*disparity_cost).boxFilter(param.kernel_radius);
        Image mean_image1_g_disparity_cost = (im1G*disparity_cost).boxFilter(param.kernel_radius);
        Image mean_image1_b_disparity_cost = (im1B*disparity_cost).boxFilter(param.kernel_radius);

        Image covariance_image1_r_disparity_cost =
            mean_image1_r_disparity_cost - meanIm1R*mean_disparity_cost;
        Image covariance_image1_g_disparity_cost =
            mean_image1_g_disparity_cost - meanIm1G*mean_disparity_cost;
        Image covariance_image1_b_disparity_cost =
            mean_image1_b_disparity_cost - meanIm1B*mean_disparity_cost;

        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++) {
                float S1[3*3] = {
                    varIm1RR(x,y)+param.epsilon, varIm1RG(x,y), varIm1RB(x,y),
                    varIm1RG(x,y), varIm1GG(x,y)+param.epsilon, varIm1GB(x,y),
                    varIm1RB(x,y), varIm1GB(x,y), varIm1BB(x,y)+param.epsilon };
                float S2[3*3];
                inverse3(S1, S2);

                tmpIm1R(x,y) = covariance_image1_r_disparity_cost(x,y) * S2[0] +
                               covariance_image1_g_disparity_cost(x,y) * S2[3] +
                               covariance_image1_b_disparity_cost(x,y) * S2[6];
                tmpIm1G(x,y) = covariance_image1_r_disparity_cost(x,y) * S2[1] +
                               covariance_image1_g_disparity_cost(x,y) * S2[4] +
                               covariance_image1_b_disparity_cost(x,y) * S2[7];
                tmpIm1B(x,y) = covariance_image1_r_disparity_cost(x,y) * S2[2] +
                               covariance_image1_g_disparity_cost(x,y) * S2[5] +
                               covariance_image1_b_disparity_cost(x,y) * S2[8];
            }
        Image tmpIm2 = (mean_disparity_cost
                        -tmpIm1R*meanIm1R
                        -tmpIm1G*meanIm1G
                        -tmpIm1B*meanIm1B).boxFilter(param.kernel_radius);

        Image tmpIm1 = tmpIm1R.boxFilter(param.kernel_radius)*im1R
                     + tmpIm1G.boxFilter(param.kernel_radius)*im1G
                     + tmpIm1B.boxFilter(param.kernel_radius)*im1B;

        tmpIm1 = tmpIm1+tmpIm2;

        // Winner takes all label selection
        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++)
                if(cost(x,y) >= tmpIm1(x,y)) {
                    cost(x,y) = tmpIm1(x,y);
                    disparity(x,y) = d;
                }
    }
    std::cout << std::endl;
    return disparity;
}
