/**
 * @file costVolume.cpp
 * @brief Disparity cost volume filtering by guided filter
 * @author Pauline Tan <pauline.tan@ens-cachan.fr>
 *         Pascal Monasse <monasse@imagine.enpc.fr>
 * 
 * Copyright (c) 2012-2013, Pauline Tan, Pascal Monasse
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Pulic License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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

/// Covariance of patches of radius \a r between images, eq. (14).
static Image covariance(Image im1, Image mean1, Image im2, Image mean2, int r) {
    return (im1*im2).boxFilter(r) - mean1*mean2;
}

/// Compute color cost according to eq. (3).
///
/// Upper-bounded (by \a maxCost) average of color absolute differences at
/// pixels im1(x,y) and im2(x+d,y).
inline float cost_color(Image im1R, Image im1G, Image im1B,
                        Image im2R, Image im2G, Image im2B,
                        int x, int y, int d, float maxCost) {
    float col1[3] = {im1R(x,y), im1G(x,y), im1B(x,y)};
    float col2[3] = {im2R(x+d,y), im2G(x+d,y), im2B(x+d,y)};
    float cost=0;
    for(int i=0; i<3; i++) { // Eq. (2)
        float tmp = col1[i]-col2[i];
        if(tmp<0) tmp=-tmp;
        cost += tmp;
    }
    cost /= 3;
    if(cost > maxCost) // Eq. (3)
        cost = maxCost;
    return cost;
}

/// Compute gradient cost according to eq. (6).
///
/// Upper-bounded (by \a maxCost) x-derivative difference at
/// pixels im1(x,y) and im2(x+d,y).
inline float cost_gradient(Image gradient1, Image gradient2,
                           int x, int y, int d, float maxCost) {
    float cost = gradient1(x,y)-gradient2(x+d,y); // Eq. (5)
    if(cost < 0)
        cost = -cost;
    if(cost > maxCost) // Eq. (6)
        cost = maxCost;
    return cost;
}

/// Compute image of matching costs at disparity \a d.
///
/// At each pixel, a linear combination of colors L1 distance (with max
/// threshold) and x-derivatives absolute difference (with max threshold).
static void compute_cost(Image im1R, Image im1G, Image im1B,
                         Image im2R, Image im2G, Image im2B,
                         Image gradient1, Image gradient2,
                         int d, const ParamGuidedFilter& param,
                         Image& cost) {
    const int width=im1R.width(), height=im1R.height();
    for(int y=0; y<height; y++)
        for(int x=0; x<width; x++) {
            float costColor = param.color_threshold; // Color L1 distance
            float costGrad = param.gradient_threshold; // x-deriv abs diff
            if(0<=x+d && x+d<width) {
                costColor = cost_color(im1R, im1G, im1B, im2R, im2G, im2B,
                                       x, y, d, param.color_threshold);
                costGrad = cost_gradient(gradient1, gradient2,
                                         x, y, d, param.gradient_threshold);
            }
            // Combination of the two penalties, eq. (7)
            cost(x,y) = (1-param.alpha)*costColor + param.alpha*costGrad;
        }
}

/// Cost volume filtering
Image filter_cost_volume(Image im1Color, Image im2Color,
                         int dispMin, int dispMax,
                         const ParamGuidedFilter& param) {
    Image im1R=im1Color.r(), im1G=im1Color.g(), im1B=im1Color.b();
    Image im2R=im2Color.r(), im2G=im2Color.g(), im2B=im2Color.b();
    const int width=im1R.width(), height=im1R.height();
    const int r = param.kernel_radius;
    std::cout << "Cost-volume: " << (dispMax-dispMin+1) << " disparities. ";

    Image disparity(width,height);
    std::fill_n(&disparity(0,0), width*height, static_cast<float>(dispMin-1));
    Image cost(width,height);
    std::fill_n(&cost(0,0), width*height, std::numeric_limits<float>::max());

    Image im1Gray(width,height);
    Image im2Gray(width,height);
    rgb_to_gray(&im1R(0,0),&im1G(0,0),&im1B(0,0), width,height, &im1Gray(0,0));
    rgb_to_gray(&im2R(0,0),&im2G(0,0),&im2B(0,0), width,height, &im2Gray(0,0));
    Image gradient1 = im1Gray.gradX();
    Image gradient2 = im2Gray.gradX();

    // Compute the mean and variance of each patch, eq. (14)
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
    for(int d=dispMin; d<=dispMax; d++) {
        std::cout << '*' << std::flush;
        compute_cost(im1R,im1G,im1B, im2R,im2G,im2B, gradient1, gradient2,
                     d, param, dCost);
        Image meanCost = dCost.boxFilter(r); // Eq. (14)

        Image covarIm1RCost = covariance(im1R, meanIm1R, dCost, meanCost, r);
        Image covarIm1GCost = covariance(im1G, meanIm1G, dCost, meanCost, r);
        Image covarIm1BCost = covariance(im1B, meanIm1B, dCost, meanCost, r);

        for(int y=0; y<height; y++)
            for(int x=0; x<width; x++) {
                // Computation of (Sigma_k+\epsilon Id)^{-1}
                float S1[3*3] = { // Eq. (21)
                    varIm1RR(x,y)+param.epsilon, varIm1RG(x,y), varIm1RB(x,y),
                    varIm1RG(x,y), varIm1GG(x,y)+param.epsilon, varIm1GB(x,y),
                    varIm1RB(x,y), varIm1GB(x,y), varIm1BB(x,y)+param.epsilon };
                float S2[3*3];
                inverseSym3(S1, S2);
                // Eq. (19)
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
                    disparity(x,y) = static_cast<float>(d);
                }
    }
    std::cout << std::endl;
    return disparity;
}
