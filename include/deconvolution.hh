#pragma once

#include "bitmap_image.hpp"
#include <vector>
#include <cmath>

class Deconvolver {
public:
    // Constructor to initialize the parameters
    Deconvolver(const std::vector<std::vector<double>>& kernel);

    Deconvolver(const std::vector<std::vector<double>>& kernel, bitmap_image image);

    Deconvolver(const std::vector<std::vector<double>> &kernel, const std::string &filePath);

// Load the image
    void loadImage(const std::string& filePath);
    void loadImaged(const bitmap_image& image);

        // Save the image
    void saveImage(const std::string& filePath);
    // Perform deconvolution
    bitmap_image getImage() { return image; }
    void deconvolve(int iterations);
    // Compute the difference between the original and deconvolved image

    bitmap_image image;
private:
    std::vector<std::vector<double>> kernel;
    std::vector<std::vector<double>> convolve(const std::vector<std::vector<double>>& image, const std::vector<std::vector<double>>& kernel);

};

