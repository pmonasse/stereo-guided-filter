#ifndef OCCLUSION_H
#define OCCLUSION_H

class Image;

/// Parameters for filling occlusions
struct ParamOcclusion {
    float sigma_space;
    float sigma_color;
    int median_radius;

    // Constructor with default parameters
    ParamOcclusion()
    : sigma_space(9),
      sigma_color(255*0.1f),
      median_radius(19) {}
};

void detect_occlusion(Image& disparityLeft, const Image& disparityRight,
                      int dOcclusion);
void fill_occlusion(const Image& dispDense, const Image& guidance,
                    Image& disparity, int dispMin, int dispMax,
                    const ParamOcclusion& paramOcc);
#endif
