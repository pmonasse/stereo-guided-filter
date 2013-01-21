#include "image.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>

/// Fill pixels below value \a vMin with max of values at closest pixels on same
/// line above \a vMin.
void Image::fillMaxX(float vMin) {
    for(int y=0; y<h; y++) {
        int x0=-1;
        float v0 = vMin;
        while(x0<w) {
            int x1=x0+1;
            while(x1<w && (*this)(x1,y)<vMin) ++x1;
            float v=v0;
            if(x1<w)
                v = std::max(v,v0=(*this)(x1,y));
            std::fill(&(*this)(x0+1,y), &(*this)(x1,y), v);
            x0=x1;
        }
    }
}

/// Derivative along x-axis
Image Image::gradX() const {
    assert(w>=2);
    Image D(w,h);
    float* out=D.tab;
    for(int y=0; y<h; y++) {
        const float* in=tab+y*w;
        *out++ = in[1]-in[0];
        for(int x=1; x+1<w; x++, in++)
            *out++ = .5f*(in[2]-in[0]);
        *out++ = in[1]-in[0];
    }
    return D;
}

/// Averaging filter with box of \a radius
Image Image::boxFilter(int radius) const {
    Image tmp(clone());

    //cumulative sum table
    for(int y=0; y<h; y++) { //horizontal
        float *in=tmp.tab+y*w, *out=in+1;
        for(int x=1; x<w; x++)
            *out++ += *in++;
    }
    for(int y=1; y<h; y++) { //vertical
        float *in=tmp.tab+(y-1)*w, *out=in+w;
        for(int x=0; x<w; x++)
            *out++ += *in++;
    }

    //box filter
    Image B(w,h);
    //cumulative sum table
    float *out=B.tab;
    for(int y=0; y<h; y++) {
        int ymin = std::max(-1, y-radius-1);
        int ymax = std::min(h-1, y+radius);
        for(int x=0; x<w; x++, out++) {
            int xmin = std::max(-1, x-radius-1);
            int xmax = std::min(w-1, x+radius);
            *out = tmp(xmax,ymax);
            if(xmin>=0)
                *out -= tmp(xmin,ymax);
            if(ymin>=0)
                *out -= tmp(xmax,ymin);
            if(xmin>=0 && ymin>=0)
                *out += tmp(xmin,ymin);
            *out /= (xmax-xmin)*(ymax-ymin);
        }
    }
    return B;
}

/// Median filter, write results in \a M
void Image::median(int radius, Image& M) const {
    int size=2*radius+1;
    size *= size;
    float* v = new float[size];
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            int n=0;
            for(int j=-radius; j<=radius; j++)
                if(0<=j+y && j+y<h)
                    for(int i=-radius; i<=radius; i++)
                        if(0<=i+x && i+x<w)
                            v[n++] = (*this)(i+x,j+y);
            std::nth_element(v, v+n/2, v+n);
            M(x,y) = v[n/2];
        }
    delete [] v;
}

/// Median filter for a color image
Image Image::medianColor(int radius) const {
    Image M(w,3*h);
    M.h=h;
    Image I=M.r(); r().median(radius, I);
    I=M.g(); g().median(radius, I);
    I=M.b(); b().median(radius, I);
    return M;
}

/// Square function
inline float sqr(float v1) {
    return v1*v1;
}

/// Square L2 distance between colors at (x1,y1) and at (x2,y2)
float Image::dist2Color(int x1,int y1, int x2,int y2) const {
    return sqr((*this)(x1,y1    )-(*this)(x2,y2    ))+
        sqr((*this)(x1,y1+  h)-(*this)(x2,y2+  h))+
        sqr((*this)(x1,y1+2*h)-(*this)(x2,y2+2*h));
}

/// @brief Weighted median filter of current image.
///
/// Image is assumed to have integer values in [vMin,vMax]. Weight are computed
/// as in bilateral filter in color image \a guidance. Only pixels of image
/// \a where outside [vMin,vMax] are filtered.
Image Image::weightedMedianColor(const Image& guidance,
                                 const Image& where, int vMin, int vMax,
                                 int radius, float sSpace, float sColor) const
{
    sSpace = 1.0f/(sSpace*sSpace);
    sColor = 1.0f/(sColor*sColor);

    const int size=vMax-vMin+1;
    std::vector<float> tab(size);
    Image M(w,h);

#ifdef _OPENMP
#pragma omp parallel for firstprivate(tab)
#endif
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            if(where(x,y)>=vMin) {
                M(x,y)=(*this)(x,y);
                continue;
            }
            std::fill(tab.begin(), tab.end(), 0);

            float sum=0;
            for(int dy=-radius; dy<=radius; dy++)
                if(0<=y+dy && y+dy<h)
                    for(int dx=-radius; dx<=radius; dx++)
                        if(0<=x+dx && x+dx<w) {
                            float w =
                                exp(-(dx*dx+dy*dy)*sSpace
                                    -guidance.dist2Color(x,y,x+dx,y+dy)*sColor);
                            tab[(int)((*this)(x+dx,y+dy))-vMin] += w;
                            sum += w;
                        }
            int d=-1;
            float cumul=0;
            sum /= 2;
            while(cumul<sum)
                cumul += tab[++d];
            M(x,y) = vMin+d;
        }
    return M;
}
