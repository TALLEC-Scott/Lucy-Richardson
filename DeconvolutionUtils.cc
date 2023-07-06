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

std::vector<std::vector<double>> DeconvolutionUtils::computeGradientX(const std::vector<std::vector<double>> &image) {
    std::vector<std::vector<double>> gradientX(image.size(), std::vector<double>(image[0].size(), 0.0));

    for (std::size_t x = 0; x < image.size(); x++) {
        for (std::size_t y = 0; y < image[x].size(); y++) {
            if (x == 0)
                gradientX[x][y] = image[x + 1][y] - image[x][y];
            else if (x == image.size() - 1)
                gradientX[x][y] = image[x][y] - image[x - 1][y];
            else
                gradientX[x][y] = (image[x + 1][y] - image[x - 1][y]) / 2.0;
        }
    }

    return gradientX;
}

std::vector<std::vector<double>> DeconvolutionUtils::computeGradientY(const std::vector<std::vector<double>> &image) {
    std::vector<std::vector<double>> gradientY(image.size(), std::vector<double>(image[0].size(), 0.0));

    for (std::size_t x = 0; x < image.size(); x++) {
        for (std::size_t y = 0; y < image[x].size(); y++) {
            if (y == 0)
                gradientY[x][y] = image[x][y + 1] - image[x][y];
            else if (y == image[x].size() - 1)
                gradientY[x][y] = image[x][y] - image[x][y - 1];
            else
                gradientY[x][y] = (image[x][y + 1] - image[x][y - 1]) / 2.0;
        }
    }

    return gradientY;
}