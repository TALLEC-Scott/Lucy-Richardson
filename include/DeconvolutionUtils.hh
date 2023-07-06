//
// Created by scott on 05/07/23.
//
#pragma once
#include "bitmap_image.hpp"

class DeconvolutionUtils {
public:
    static void computeDifference(const bitmap_image& blurredImage, const bitmap_image& unblurredImage, bitmap_image& differenceImage);

    static void applyGrayscalePrior(bitmap_image &differenceImage);
    static std::vector<std::vector<double>> computeGradientX(const std::vector<std::vector<double>>& image);
    static std::vector<std::vector<double>> computeGradientY(const std::vector<std::vector<double>>& image);

    };

