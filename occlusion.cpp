#include "occlusion.h"
#include "image.h"

/// Detect left-right discrepancies in disparity and put incoherent pixels to
/// value \a dOcclusion in \a disparityLeft.
void detect_occlusion(Image& disparityLeft, const Image& disparityRight,
                      int dOcclusion) {
    const int w=disparityLeft.width(), h=disparityLeft.height();
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            int d = (int)disparityLeft(x,y);
            if(x+d<0 || x+d>=w ||
               disparityLeft(x,y) != -disparityRight(x+d,y))
                disparityLeft(x,y) = dOcclusion;
        }
}

/// Fill occlusions by weighted median filtering.
///
/// \param dispDense Disparity image
/// \param guidance Color guidance image, where weights are computed
/// \param disparity Values outside [dispMin,dispMax] are interpolated
/// \param dispMin,dispMax Min/max disparities
/// \param paramOcc Parameters to compute weights in bilateral filtering
void fill_occlusion(const Image& dispDense, const Image& guidance,
                    Image& disparity, int dispMin, int dispMax,
                    const ParamOcclusion& paramOcc) {
    disparity = dispDense.weightedMedianColor(guidance,
                                              disparity, dispMin, dispMax, 
                                              paramOcc.median_radius,
                                              paramOcc.sigma_space,
                                              paramOcc.sigma_color);
}
