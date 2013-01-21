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

static void usage() {
    ParamCVFilter p;
    ParamOcclusion q;
    std::cerr << "Fast Cost-Volume Filtering for Visual Correspondence\n"
              << "Usage: ./test [options] im1.png im2.png dmin dmax\n\n"
              << "Options (default values in parentheses)\n"
              << "Cost-volume filtering parameters:\n"
              << "    -R radius: radius of the guided filter ("
              <<p.kernel_radius << ")\n"
              << "    -A alpha: value of alpha ("<<p.alpha<<")\n"
              << "    -E epsilon: regularization parameter ("<<p.epsilon <<")\n"
              << "    -C tau1: max for color difference ("
              <<p.color_threshold<<")\n"
              << "    -G tau2: max for gradient difference ("
              <<p.gradient_threshold<<")\n\n"
              << "Occlusion detection:\n"
              << "    -o: detect occlusion\n\n"
              << "Densification:\n"
              << "    -O: fill occlusion and launch the post-processing\n"
              << "    -r radius: radius of the weighted median filter ("
              <<q.median_radius << ")\n"
              << "    -c sigmac: value of sigma_color ("
              <<q.sigma_color << ")\n"
              << "    -s sigmas: value of sigma_space ("
              <<q.sigma_space << ")"
              << std::endl;
}

int main(int argc, char *argv[])
{
    CmdLine cmd;

    ParamCVFilter paramCV; // Parameters for cost-volume filtering
    cmd.add( make_option('R',paramCV.kernel_radius) );
    cmd.add( make_option('A',paramCV.alpha) );
    cmd.add( make_option('E',paramCV.epsilon) );
    cmd.add( make_option('C',paramCV.color_threshold) );
    cmd.add( make_option('G',paramCV.gradient_threshold) );

    ParamOcclusion paramOcc; // Parameters for filling occlusions
    cmd.add( make_switch('o') ); // Detect occlusion
    cmd.add( make_switch('O') ); // Fill occlusion
    cmd.add( make_option('r',paramOcc.median_radius) );
    cmd.add( make_option('c',paramOcc.sigma_color) );
    cmd.add( make_option('s',paramOcc.sigma_space) );

    try {
        cmd.process(argc, argv);
    } catch(std::string str) {
        std::cerr << "Error: " << str << std::endl<<std::endl;
        usage();
        return 1;
    }
    if(argc!=5) {
        usage();
        return 1;
    }
    bool detectOcc = cmd.used('o') || cmd.used('O');
    bool fillOcc = cmd.used('O');

    // Load images and convert to grayscale
    size_t width, height, width2, height2;
    float* pix1 = read_png_f32_rgb(argv[1], &width, &height);
    float* pix2 = read_png_f32_rgb(argv[2], &width2, &height2);
    if(width != width2 || height != height2) {
        std::cerr << "The images must have the same size!" << std::endl;
        return 1;
    }
    Image im1(pix1, width, height);
    Image im2(pix2, width, height);

    // Set disparity range
    int dispMin, dispMax;
    if(! ((std::istringstream(argv[3])>>dispMin).eof() &&
          (std::istringstream(argv[4])>>dispMax).eof())) {
        std::cerr << "Error reading dMin or dMax" << std::endl;
        return 1;
    }
    if(dispMin>dispMax) {
        std::cerr << "Wrong disparity range! (dispMin > dispMax)" << std::endl;
        return 1;
    }

    Image disparity = filter_cost_volume(im1, im2, dispMin, dispMax, paramCV);
    if(! save_disparity(OUTFILE1, disparity, dispMin, dispMax)) {
        std::cerr << "Error writing file " << OUTFILE1 << std::endl;
        return 1;
    }

    if(detectOcc) {
        std::cout << "Detect occlusions...";
        Image disparity2= filter_cost_volume(im2,im1,-dispMax,-dispMin,paramCV);
        detect_occlusion(disparity, disparity2, dispMin-1);
        if(! save_disparity(OUTFILE2, disparity, dispMin, dispMax))  {
            std::cerr << "Error writing file " << OUTFILE2 << std::endl;
            return 1;
        }
    }

    if(fillOcc) {
        std::cout << "Post-processing: fill occlusions" << std::endl;
        Image dispDense = disparity.clone();
        dispDense.fillMaxX(dispMin);
        if(! save_disparity(OUTFILE3, dispDense, dispMin, dispMax)) {
            std::cerr << "Error writing file " << OUTFILE3 << std::endl;
            return 1;
        }

        std::cout << "Post-processing: smooth the disparity map" << std::endl;
        fill_occlusion(dispDense, im1.medianColor(1),
                       disparity, dispMin, dispMax, paramOcc);
        if(! save_disparity(OUTFILE4, disparity, dispMin, dispMax)) {
            std::cerr << "Error writing file " << OUTFILE4 << std::endl;
            return 1;
        }
    }

    free(pix1);
    free(pix2);
    return 0;
}
