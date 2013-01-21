#include "costVolume.h"
#include "image.h"
#include "io_png.h"
#include <algorithm>
#include <limits>
#include <iostream>

/// Inverse of symmetric 3x3 matrix
static void inverseSym3(const float* matrix, float* inverse) {
    inverse[0] = matrix[4]*matrix[8] - matrix[5]*matrix[7];
    inverse[1] = matrix[2]*matrix[7] - matrix[1]*matrix[8];
    inverse[2] = matrix[1]*matrix[5] - matrix[2]*matrix[4];
    float det = matrix[0]*inverse[0]+matrix[3]*inverse[1]+matrix[6]*inverse[2];
    det = 1/det;
    inverse[0] *= det;
    inverse[1] *= det;
    inverse[2] *= det;
    inverse[3] = inverse[1];
    inverse[4] = (matrix[0]*matrix[8] - matrix[2]*matrix[6]) * det;
    inverse[5] = (matrix[2]*matrix[3] - matrix[0]*matrix[5]) * det;
    inverse[6] = inverse[2];
    inverse[7] = inverse[5];
    inverse[8] = (matrix[0]*matrix[4] - matrix[1]*matrix[3]) * det;
}

/// Covariance of patches of radius \a r between images.
static Image covariance(Image im1, Image mean1, Image im2, Image mean2, int r) {
    return (im1*im2).boxFilter(r) - mean1*mean2;
}

/// Compute image of matching costs at disparity \a d.
///
/// At each pixel, a linear combination of colors L1 distance (with max
/// threshold) and x-derivatives absolute difference (with max threshold).
static void compute_cost(Image im1R, Image im1G, Image im1B,
                         Image im2R, Image im2G, Image im2B,
                         Image gradient1, Image gradient2,
                         int d, const ParamCVFilter& param,
                         Image& cost) {
    const int width=im1R.width(), height=im1R.height();
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
            float costColor = param.color_threshold; // Color L1 distance
            float costGradient = param.gradient_threshold; // x-deriv abs diff
            if(0<=x+d && x+d<width) {
                float col1[3] = {im1R(x,y), im1G(x,y), im1B(x,y)};
                float col2[3] = {im2R(x+d,y), im2G(x+d,y), im2B(x+d,y)};
                costColor=0;
                for(int i=0; i<3; i++) {
                    float tmp = col1[i]-col2[i];
                    if(tmp<0) tmp=-tmp;
                    costColor += tmp;
                }
                costColor /= 3;
                if(costColor > param.color_threshold) 
                    costColor = param.color_threshold;

                costGradient = gradient1(x,y)-gradient2(x+d,y);
                if(costGradient < 0)
                    costGradient = -costGradient;
                if(costGradient > param.gradient_threshold) 
                    costGradient = param.gradient_threshold;
            }

            // Combination of the two penalties
            cost(x,y) = (1-param.alpha)*costColor + param.alpha*costGradient;
        }
}

/// Cost volume filtering
Image filter_cost_volume(Image im1Color, Image im2Color,
                         int dispMin, int dispMax,
                         const ParamCVFilter& param) {
    Image im1R=im1Color.r(), im1G=im1Color.g(), im1B=im1Color.b();
    Image im2R=im2Color.r(), im2G=im2Color.g(), im2B=im2Color.b();
    const int width=im1R.width(), height=im1R.height();
    const int r = param.kernel_radius;
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
    Image meanIm1R = im1R.boxFilter(r);
    Image meanIm1G = im1G.boxFilter(r);
    Image meanIm1B = im1B.boxFilter(r);

    Image varIm1RR = covariance(im1R, meanIm1R, im1R, meanIm1R, r);
    Image varIm1RG = covariance(im1R, meanIm1R, im1G, meanIm1G, r);
    Image varIm1RB = covariance(im1R, meanIm1R, im1B, meanIm1B, r);
    Image varIm1GG = covariance(im1G, meanIm1G, im1G, meanIm1G, r);
    Image varIm1GB = covariance(im1G, meanIm1G, im1B, meanIm1B, r);
    Image varIm1BB = covariance(im1B, meanIm1B, im1B, meanIm1B, r);

    Image aR(width,height),aG(width,height),aB(width,height);
    Image dCost(width,height);
    for(int d=dispMin; d<dispMax+1; d++) {
        std::cout << '*' << std::flush;
        compute_cost(im1R,im1G,im1B, im2R,im2G,im2B, gradient1, gradient2,
                     d, param, dCost);
        Image meanCost = dCost.boxFilter(r);

        Image covarIm1RCost = covariance(im1R, meanIm1R, dCost, meanCost, r);
        Image covarIm1GCost = covariance(im1G, meanIm1G, dCost, meanCost, r);
        Image covarIm1BCost = covariance(im1B, meanIm1B, dCost, meanCost, r);

        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++) {
                float S1[3*3] = {
                    varIm1RR(x,y)+param.epsilon, varIm1RG(x,y), varIm1RB(x,y),
                    varIm1RG(x,y), varIm1GG(x,y)+param.epsilon, varIm1GB(x,y),
                    varIm1RB(x,y), varIm1GB(x,y), varIm1BB(x,y)+param.epsilon };
                float S2[3*3];
                inverseSym3(S1, S2);

                aR(x,y) = covarIm1RCost(x,y) * S2[0] +
                          covarIm1GCost(x,y) * S2[1] +
                          covarIm1BCost(x,y) * S2[2];
                aG(x,y) = covarIm1RCost(x,y) * S2[3] +
                          covarIm1GCost(x,y) * S2[4] +
                          covarIm1BCost(x,y) * S2[5];
                aB(x,y) = covarIm1RCost(x,y) * S2[6] +
                          covarIm1GCost(x,y) * S2[7] +
                          covarIm1BCost(x,y) * S2[8];
            }
        Image b = (meanCost-aR*meanIm1R-aG*meanIm1G-aB*meanIm1B).boxFilter(r);
        b += aR.boxFilter(r)*im1R+aG.boxFilter(r)*im1G+aB.boxFilter(r)*im1B;

        // Winner takes all label selection
        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++)
                if(cost(x,y) >= b(x,y)) {
                    cost(x,y) = b(x,y);
                    disparity(x,y) = d;
                }
    }
    std::cout << std::endl;
    return disparity;
}
