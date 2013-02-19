/**
 * @file costVolume.h
 * @brief Disparity cost volume filtering by guided filter
 * @author Pauline Tan <pauline.tan@ens-cachan.fr>
 *         Pascal Monasse <monasse@imagine.enpc.fr>
 * 
 * Copyright (c) 2012-2013, Pauline Tan, Pascal Monasse
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

#ifndef COSTVOLUME_H
#define COSTVOLUME_H

class Image;

/// Parameters specific to the guided filter
struct ParamGuidedFilter {
    float color_threshold;
    float gradient_threshold;
    float alpha;
    int kernel_radius;
    float epsilon;

    /// Constructor with default parameters
    ParamGuidedFilter()
    : color_threshold(7),
      gradient_threshold(2),
      alpha(1-0.1f),
      kernel_radius(9),
      epsilon(0.0001*255*255) {}
};

Image filter_cost_volume(Image im1Color, Image im2Color,
                         int dispMin, int dispMax,
                         const ParamGuidedFilter& param);

#endif
