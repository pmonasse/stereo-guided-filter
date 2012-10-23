Fast Cost-Volume Filtering for Visual Correspondence

Usage: ./test [options] -- im1.png im2.png dmin dmax

Options
Cost-volume filtering parameters:
    -R radius: radius of the guided filter
    -A alpha: value of alpha
    -E epsilon: value of the regularization parameter
    -C tau1: threshold for the color difference
    -G tau2: threshold for the gradient difference
    -B B: value of the extension constant
    
Occlusion detection:
    -o: detect occlusion
    
Densification:
    -O: fill occlusion and launch the post-processing
    -r radius: radius of the weighted median filter
    -c sigmac: value of sigma_c
    -s sigmas: value of sigma_s
    
