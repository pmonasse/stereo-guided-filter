Fast Cost-Volume Filtering for Visual Correspondence

- Build
mkdir Build && cd Build
cmake -D CMAKE_BUILD_TYPE:string=Release ..
make

- Run
Usage: ./costVolumeFilter [options] im1.png im2.png dmin dmax

Options (default values in parentheses)
Cost-volume filtering parameters:
    -R radius: radius of the guided filter (9)
    -A alpha: value of alpha (0.9)
    -E epsilon: regularization parameter (6.5025)
    -C tau1: max for color difference (7)
    -G tau2: max for gradient difference (2)

Occlusion detection:
    -o tolDiffDisp: tolerance for left-right disp. diff. (0)

Densification:
    -O: fill occlusion and launch the post-processing
    -r radius: radius of the weighted median filter (19)
    -c sigmac: value of sigma_color (25.5)
    -s sigmas: value of sigma_space (9)

    -a grayMin: value of gray for min disparity (255)
    -b grayMax: value of gray for max disparity (0)

- Output image files
disparity.png: disparity map after cost-volume filtering
disparity_occlusion.png: after left-right check
disparity_occlusion_filled.png: simple densification
disparity_occlusion_filled_smoothed.png: final densification with median filter
