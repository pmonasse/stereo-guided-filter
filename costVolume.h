#ifndef COSTVOLUME_H
#define COSTVOLUME_H

class Image;

struct ParamCVFilter {
    float color_threshold;
    float gradient_threshold;
    float alpha;
    int kernel_radius;
    float epsilon;
    float border_threshold;

    /// Constructor with default parameters
    ParamCVFilter()
    : color_threshold(7),
      gradient_threshold(2),
      alpha(1-0.1f),
      kernel_radius(9),
      epsilon(0.0001*255*255),
      border_threshold(3) {}
};

Image filter_cost_volume(Image im1Color, Image im2Color,
                         int dispMin, int dispMax,
                         const ParamCVFilter& param);

#endif
