/**
 * @file image.h
 * @brief image class with shallow copy
 * @author Pascal Monasse <monasse@imagine.enpc.fr>
 * 
 * Copyright (c) 2012-2013, Pascal Monasse
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under, at your option, the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version, or the terms of the 
 * simplified BSD license.
 *
 * You should have received a copy of these licenses along with this program.
 * If not, see <http://www.gnu.org/licenses/> and
 * <http://www.opensource.org/licenses/bsd-license.html>.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

/// Float image class, with shallow copy for performance.
///
/// Copy constructor and operator= perform a shallow copy, so pixels are shared.
/// To perform a deep copy, use method clone().
/// There is a constructor taking array of pixels; no copy is done, make sure
/// the array exists during the lifetime of the image.
/// The methods using color image assume consecutive channels (no interlace).
class Image {
    int* count;
    float* tab;
    int w, h;
    void kill();
public:
    Image(int width, int height);
    Image(float* pix, int width, int height);
    Image(const Image& I);
    ~Image() { kill(); }
    Image& operator=(const Image& I);
    Image clone() const;

    int width() const { return w; }
    int height() const { return h; }
    float  operator()(int i,int j) const { return tab[j*w+i]; }
    float& operator()(int i,int j)       { return tab[j*w+i]; }

    Image r() const { return Image(tab+0*w*h,w,h); }
    Image g() const { return Image(tab+1*w*h,w,h); }
    Image b() const { return Image(tab+2*w*h,w,h); }

    Image operator+(const Image& I) const;
    Image operator-(const Image& I) const;
    Image operator*(const Image& I) const;
    Image& operator+=(const Image& I);

    // Filters (implemented in filters.cpp)
    Image gradX() const;
    void fillMinX(float vMin);
    void fillMaxX(float vMin);
    Image boxFilter(int radius) const;
    void median(int radius, Image& M) const;
    Image medianColor(int radius) const;
    Image weightedMedianColor(const Image& guidance,
                              const Image& where, int vMin, int vMax,
                              int radius,
                              float sigmaSpace, float sigmaColor) const;
private:
    void fillX(float vMin, const float& (*cmp)(const float&,const float&));
    float dist2Color(int x1,int y1, int x2,int y2) const;
    void weighted_histo(std::vector<float>& tab, int x, int y, int radius,
                        float vMin, const Image& guidance,
                        float sSpace, float sColor) const;
};

bool save_disparity(const char* file_name, const Image& disparity,
                    int dMin, int dMax, int grayMin, int grayMax);

#endif
