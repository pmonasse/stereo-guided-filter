/**
 * @file main.cpp
 * @brief Disparity map estimation through cost-volume filtering
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

#include "costVolume.h"
#include "occlusion.h"
#include "image.h"
#include "cmdLine.h"
#include "io_png.h"
#include <iostream>

/// Names of output image files
static const char* OUTFILE1="disparity.png";
static const char* OUTFILE2="disparity_occlusion.png";
static const char* OUTFILE3="disparity_occlusion_filled.png";
static const char* OUTFILE4="disparity_occlusion_filled_smoothed.png";

static void usage(const char* name) {
    ParamGuidedFilter p;
    ParamOcclusion q;
    std::cerr <<"Stereo Disparity through Cost Aggregation with Guided Filter\n"
              << "Usage: " << name << " [options] im1.png im2.png dmin dmax\n\n"
              << "Options (default values in parentheses)\n"
              << "Cost-volume filtering parameters:\n"
              << "    -R radius: radius of the guided filter ("
              <<p.kernel_radius << ")\n"
              << "    -A alpha: value of alpha ("<<p.alpha<<")\n"
              << "    -E epsilon: regularization parameter ("<<p.epsilon <<")\n"
              << "    -C tau1: max for color difference ("
              <<p.color_threshold<<")\n"
              << "    -G tau2: max for gradient difference ("
              <<p.gradient_threshold << ")\n\n"
              << "Occlusion detection:\n"
              << "    -o tolDiffDisp: tolerance for left-right disp. diff. ("
              <<q.tol_disp << ")\n\n"
              << "Densification:\n"
              << "    -O sense: fill occlusion, sense='r':right, 'l':left\n"
              << "    -r radius: radius of the weighted median filter ("
              <<q.median_radius << ")\n"
              << "    -c sigmac: value of sigma_color ("
              <<q.sigma_color << ")\n"
              << "    -s sigmas: value of sigma_space ("
              <<q.sigma_space << ")\n\n"
              << "    -a grayMin: value of gray for min disparity (255)\n"
              << "    -b grayMax: value of gray for max disparity (0)"
              << std::endl;
}

int main(int argc, char *argv[])
{
    int grayMin=255, grayMax=0;
    char sense='r'; // Camera motion direction: 'r'=to-right, 'l'=to-left
    CmdLine cmd;

    ParamGuidedFilter paramGF; // Parameters for cost-volume filtering
    cmd.add( make_option('R',paramGF.kernel_radius) );
    cmd.add( make_option('A',paramGF.alpha) );
    cmd.add( make_option('E',paramGF.epsilon) );
    cmd.add( make_option('C',paramGF.color_threshold) );
    cmd.add( make_option('G',paramGF.gradient_threshold) );

    ParamOcclusion paramOcc; // Parameters for filling occlusions
    cmd.add( make_option('o',paramOcc.tol_disp) ); // Detect occlusion
    cmd.add( make_option('O',sense) ); // Fill occlusion
    cmd.add( make_option('r',paramOcc.median_radius) );
    cmd.add( make_option('c',paramOcc.sigma_color) );
    cmd.add( make_option('s',paramOcc.sigma_space) );

    cmd.add( make_option('a',grayMin) );
    cmd.add( make_option('b',grayMax) );
    try {
        cmd.process(argc, argv);
    } catch(std::string str) {
        std::cerr << "Error: " << str << std::endl<<std::endl;
        usage(argv[0]);
        return 1;
    }
    if(argc!=5) {
        usage(argv[0]);
        return 1;
    }
    bool detectOcc = cmd.used('o') || cmd.used('O');
    bool fillOcc = cmd.used('O');

    if(sense != 'r' && sense != 'l') {
        std::cerr << "Error: invalid camera motion direction " << sense
                  << " (must be r or l)" << std::endl;
        return 1;
    }

    // Load images
    size_t width, height, width2, height2;
    float* pix1 = io_png_read_f32_rgb(argv[1], &width, &height);
    float* pix2 = io_png_read_f32_rgb(argv[2], &width2, &height2);
    if(width != width2 || height != height2) {
        std::cerr << "The images must have the same size!" << std::endl;
        return 1;
    }
    Image im1(pix1, width, height);
    Image im2(pix2, width, height);

    // Set disparity range
    int dMin, dMax;
    if(! ((std::istringstream(argv[3])>>dMin).eof() &&
          (std::istringstream(argv[4])>>dMax).eof())) {
        std::cerr << "Error reading dMin or dMax" << std::endl;
        return 1;
    }
    if(dMin>dMax) {
        std::cerr << "Wrong disparity range! (dMin > dMax)" << std::endl;
        return 1;
    }

    Image disparity = filter_cost_volume(im1, im2, dMin, dMax, paramGF);
    if(! save_disparity(OUTFILE1, disparity, dMin,dMax, grayMin,grayMax)) {
        std::cerr << "Error writing file " << OUTFILE1 << std::endl;
        return 1;
    }

    if(detectOcc) {
        std::cout << "Detect occlusions...";
        Image disparity2= filter_cost_volume(im2,im1,-dMax,-dMin,paramGF);
        detect_occlusion(disparity, disparity2, dMin-1, paramOcc.tol_disp);
        if(! save_disparity(OUTFILE2, disparity, dMin,dMax, grayMin,grayMax))  {
            std::cerr << "Error writing file " << OUTFILE2 << std::endl;
            return 1;
        }
    }

    if(fillOcc) {
        std::cout << "Post-processing: fill occlusions" << std::endl;
        Image dispDense = disparity.clone();
        if(sense == 'r')
            dispDense.fillMaxX(dMin);
        else
            dispDense.fillMinX(dMin);
        if(! save_disparity(OUTFILE3, dispDense, dMin,dMax, grayMin,grayMax)) {
            std::cerr << "Error writing file " << OUTFILE3 << std::endl;
            return 1;
        }

        std::cout << "Post-processing: smooth the disparity map" << std::endl;
        fill_occlusion(dispDense, im1.medianColor(1),
                       disparity, dMin, dMax, paramOcc);
        if(! save_disparity(OUTFILE4, disparity, dMin,dMax, grayMin,grayMax)) {
            std::cerr << "Error writing file " << OUTFILE4 << std::endl;
            return 1;
        }
    }

    free(pix1);
    free(pix2);
    return 0;
}
