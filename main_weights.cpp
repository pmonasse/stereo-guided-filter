/**
 * @file main_weights.cpp
 * @brief Show weights of the guidance filter at a point
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
#include "cmdLine.h"
#include "io_png.h"
#include <iostream>

// Functions defined below
Image cut_image(const Image& in, int& x, int& y, int radius);
Image compute_weights(const Image& in, int x, int y, int radius, float epsilon);
bool save_weights(const char* fileName,const Image& W, int grayMin,int grayMax);

static void usage(const char* name, int radius, float epsilon) {
    std::cerr <<"Stereo Disparity through Cost Aggregation with Guided Filter\n"
              << "Usage: " << name << " [options] in.png x y out.png\n\n"
              << "Options (default values in parentheses)\n"
              << "    -R radius: radius of the guided filter ("<<radius << ")\n"
              << "    -E epsilon: regularization parameter ("<<epsilon <<")\n"
              << "    -a grayMin: value of gray for min weight (0)\n"
              << "    -b grayMax: value of gray for max weight (255)"
              << std::endl;
}

int main(int argc, char *argv[])
{
    int grayMin=0, grayMax=255;
    int radius=9;
    float epsilon = 0.0001f*255*255;
    CmdLine cmd;
    cmd.add( make_option('R',radius) );
    cmd.add( make_option('E',epsilon) );
    cmd.add( make_option('a',grayMin) );
    cmd.add( make_option('b',grayMax) );
    try {
        cmd.process(argc, argv);
    } catch(std::string str) {
        std::cerr << "Error: " << str << std::endl<<std::endl;
        usage(argv[0], radius, epsilon);
        return 1;
    }
    if(argc!=5) {
        usage(argv[0], radius, epsilon);
        return 1;
    }

    // Load image
    size_t width, height;
    float* pix = io_png_read_f32_rgb(argv[1], &width, &height);
    if(!pix) {
        std::cerr << "Cannot read image file " << argv[1] << std::endl;
        return 1;
    }
    Image in(pix, width, height);

    // Get (x,y)
    int x, y;
    if(! ((std::istringstream(argv[2])>>x).eof() &&
          (std::istringstream(argv[3])>>y).eof())) {
        std::cerr << "Error reading x or y" << std::endl;
        return 1;
    }
    if(! (0<=x && (size_t)x<width && 0<=y && (size_t)y<height)) {
        std::cerr << "Error: point (x,y) must be inside the image" << std::endl;
        return 1;
    }

    in = cut_image(in, x, y, radius);

    Image w = compute_weights(in, x, y, radius, epsilon);
    if(! save_weights(argv[4], w, grayMin,grayMax)) {
        std::cerr << "Error writing file " << argv[4] << std::endl;
        return 1;
    }

    free(pix);
    delete [] &in(0,0);
    return 0;
}

/// Extract subimage [x-2*radius,x+2*radius] x [y-2*radius,y+2*radius]
Image cut_image(const Image& in, int& x, int& y, int radius) {
    int x0=x-2*radius, y0=y-2*radius;
    int x1=x+2*radius, y1=y+2*radius;
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    x -= x0;
    y -= y0;
    if(x1 >= in.width())  x1 = in.width() -1;
    if(y1 >= in.height()) y1 = in.height()-1;
    float* pix = new float[3*(x1-x0+1)*(y1-y0+1)];
    float* p=pix;
    for(int c=0; c<3; c++)
        for(int j=y0; j<=y1; j++)
            for(int i=x0; i<=x1; i++)
                *p++ = in(i,j+c*in.height());
    return Image(pix, x1-x0+1, y1-y0+1);
}

/// Return maximum value of pixel in image.
static float min(const Image& im) {
    float M=im(0,0);
    for(int j=0; j<im.height(); j++)
        for(int i=0; i<im.width(); i++)
            if( M > im(i,j) )
                M = im(i,j);
    return M;
}

/// Return maximum value of pixel in image.
static float max(const Image& im) {
    float M=im(0,0);
    for(int j=0; j<im.height(); j++)
        for(int i=0; i<im.width(); i++)
            if( M < im(i,j) )
                M = im(i,j);
    return M;
}

/// Save weight image in 8-bit PNG image.
///
/// The weight->gray function is affine: gray=a*disp+b.
/// Pixels outside [0,255] are assumed invalid and written in cyan color.
bool save_weights(const char* fileName, const Image& W, int grayMin,int grayMax)
{
    const float M = max(W), m=min(W);
    const float a=(grayMax-grayMin)/(M-m);
    const float b=(grayMin*M-grayMax*m)/(M-m);

    const int w=W.width(), h=W.height();
    const float* in=&(const_cast<Image&>(W))(0,0);
    unsigned char *out = new unsigned char[3*w*h];
    unsigned char *red=out, *green=out+w*h, *blue=out+2*w*h;
    for(size_t i=w*h; i>0; i--, in++, red++) {
        if(m<=*in && *in<=M) {
            float v = a * *in + b +0.5f;
            if(v<0) v=0;
            if(v>255) v=255;
            *red = static_cast<unsigned char>(v);
            *green++ = *red;
            *blue++  = *red;
        } else { // Cyan for disparities out of range
            *red=0;
            *green++ = *blue++ = 255;
        }
    }
    bool ok = (io_png_write_u8(fileName, out, w, h, 3) == 0);
    delete [] out;
    return ok;
}

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

/// Compute weights of guidance filter associated to (x,y).
Image compute_weights(const Image& in, int x, int y, int r, float epsilon) {
    Image R=in.r(), G=in.g(), B=in.b();
    const int width=R.width(), height=R.height();
    Image W(width,height);
    std::fill_n(&W(0,0), width*height, 0.0f);

    // Compute the mean and variance of each patch, eq. (14)
    Image meanR = R.boxFilter(r);
    Image meanG = G.boxFilter(r);
    Image meanB = B.boxFilter(r);

    Image varRR = covariance(R, meanR, R, meanR, r);
    Image varRG = covariance(R, meanR, G, meanG, r);
    Image varRB = covariance(R, meanR, B, meanB, r);
    Image varGG = covariance(G, meanG, G, meanG, r);
    Image varGB = covariance(G, meanG, B, meanB, r);
    Image varBB = covariance(B, meanB, B, meanB, r);

    Image aR(width,height),aG(width,height),aB(width,height);
    Image delta(width,height);
    for(int j=0; j<height; j++)
        for(int i=0; i<width; i++) {
            std::fill_n(&delta(0,0), width*height, 0.0f);
            delta(i,j) = 1.0f;
            Image meanCost = delta.boxFilter(r); // Eq. (14)

            Image covarRCost = covariance(R, meanR, delta, meanCost, r);
            Image covarGCost = covariance(G, meanG, delta, meanCost, r);
            Image covarBCost = covariance(B, meanB, delta, meanCost, r);

            for(int v=0; v<height; v++)
                for(int u=0; u<width; u++) {
                    // Computation of (Sigma_k+\epsilon Id)^{-1}
                    float S1[3*3] = { // Eq. (21)
                        varRR(u,v)+epsilon, varRG(u,v), varRB(u,v),
                        varRG(u,v), varGG(u,v)+epsilon, varGB(u,v),
                        varRB(u,v), varGB(u,v), varBB(u,v)+epsilon };
                    float S2[3*3];
                    inverseSym3(S1, S2);
                    // Eq. (19)
                    aR(u,v) = covarRCost(u,v) * S2[0] +
                              covarGCost(u,v) * S2[1] +
                              covarBCost(u,v) * S2[2];
                    aG(u,v) = covarRCost(u,v) * S2[3] +
                              covarGCost(u,v) * S2[4] +
                              covarBCost(u,v) * S2[5];
                    aB(u,v) = covarRCost(u,v) * S2[6] +
                              covarGCost(u,v) * S2[7] +
                              covarBCost(u,v) * S2[8];
                }
            Image b = (meanCost-aR*meanR-aG*meanG-aB*meanB).boxFilter(r);
            b += aR.boxFilter(r)*R+aG.boxFilter(r)*G+aB.boxFilter(r)*B;

            W(i,j) = b(x,y);
        }

    return W;
}
