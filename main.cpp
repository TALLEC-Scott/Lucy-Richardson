#include "blur_image.h"
#include "deconvolution.h"
#include <cmath>
#include <limits>
#include <iostream>

int main() {
    // Load an image
    bitmap_image originalImage("image.bmp");

    // Define blur types
    ImageBlurrer::BlurType blurTypes[3] = {ImageBlurrer::GAUSSIAN, ImageBlurrer::BOX, ImageBlurrer::MOTION};

    // Iterate over blur types
    for(int i=0; i<3; i++) {
        // Blur an image
        ImageBlurrer blurrer(blurTypes[i], 3, 3.0, 45.0);
        blurrer.loadImage("image.bmp");
        blurrer.blurImage();
        blurrer.addNoise(0.0, 10.0);
        std::string blurredImageFile = "blurred_image" + std::to_string(i) + ".bmp";
        blurrer.saveImage(blurredImageFile);

        // Load the blurred image
        bitmap_image blurredImage(blurredImageFile);

        // Get the PSF used to blur the image
        std::vector<std::vector<double>> kernel = blurrer.getKernel();

        // Perform the deconvolution
        Deconvolver deconvolver(kernel);
        deconvolver.loadImage(blurredImageFile);
        deconvolver.deconvolve(3);
        std::string deblurredImageFile = "deblurred_image" + std::to_string(i) + ".bmp";
        deconvolver.saveImage(deblurredImageFile);
        break;
    }

    return 0;
}
