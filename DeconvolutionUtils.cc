//
// Created by scott on 05/07/23.
//
#include "bitmap_image.hpp"
#include <cmath>
#include "DeconvolutionUtils.hh"

void DeconvolutionUtils::computeDifference(const bitmap_image& blurredImage, const bitmap_image& unblurredImage, bitmap_image& differenceImage) {
    if (blurredImage.width() != unblurredImage.width() || blurredImage.height() != unblurredImage.height()) {
        std::cerr << "Error: Blurred and unblurred images have different dimensions." << std::endl;
        return;
    }

    differenceImage = blurredImage;

    for (std::size_t x = 0; x < blurredImage.width(); ++x) {
        for (std::size_t y = 0; y < blurredImage.height(); ++y) {
            rgb_t blurredPixel = blurredImage.get_pixel(x, y);
            rgb_t unblurredPixel = unblurredImage.get_pixel(x, y);
            rgb_t diffPixel;
            diffPixel.red = std::abs(blurredPixel.red - unblurredPixel.red);
            diffPixel.green = std::abs(blurredPixel.green - unblurredPixel.green);
            diffPixel.blue = std::abs(blurredPixel.blue - unblurredPixel.blue);
            differenceImage.set_pixel(x, y, diffPixel);
        }
    }

}

void DeconvolutionUtils::applyGrayscalePrior(bitmap_image& differenceImage) {
    for (std::size_t x = 0; x < differenceImage.width(); ++x) {
        for (std::size_t y = 0; y < differenceImage.height(); ++y) {
            rgb_t pixel = differenceImage.get_pixel(x, y);
            unsigned char average = (pixel.red + pixel.green + pixel.blue) / 3;
            pixel.red = average;
            pixel.green = average;
            pixel.blue = average;
            differenceImage.set_pixel(x, y, pixel);
        }
    }
}

void
DeconvolutionUtils::computeGrayscaleDifference(const bitmap_image &blurredImage, const bitmap_image &unblurredImage,
                                               bitmap_image &differenceImage) {



}
