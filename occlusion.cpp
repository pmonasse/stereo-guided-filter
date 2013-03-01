/**
 * @file occlusion.cpp
 * @brief Detect and fill occlusions by left-right consistency
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

#include "occlusion.h"
#include "image.h"
#include <cstdlib>

/// Detect left-right discrepancies in disparity and put incoherent pixels to
/// value \a dOcclusion in \a disparityLeft.
void detect_occlusion(Image& disparityLeft, const Image& disparityRight,
                      int dOcclusion, int tolDisp) {
    const int w=disparityLeft.width(), h=disparityLeft.height();
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            int d = (int)disparityLeft(x,y);
            if(x+d<0 || x+d>=w || abs(d+(int)disparityRight(x+d,y))>tolDisp)
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
