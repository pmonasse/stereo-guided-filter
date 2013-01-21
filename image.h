#ifndef IMAGE_H
#define IMAGE_H

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
    float dist2Color(int x1,int y1, int x2,int y2) const;
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

    // Filters
    Image gradX() const;
    void fillMaxX(float vMin);
    Image boxFilter(int radius) const;
    void median(int radius, Image& M) const;
    Image medianColor(int radius) const;
    Image weightedMedianColor(const Image& guidance,
                              const Image& where, int vMin, int vMax,
                              int radius,
                              float sigmaSpace, float sigmaColor) const;
};

bool save_disparity(const char* file_name, const Image& disparity,
                    float dispMin, float dispMax);

#endif
