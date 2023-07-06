//
// Created by scott on 05/07/23.
//
#pragma once
#include "bitmap_image.hpp"

class DeconvolutionUtils {
public:
    static void computeDifference(const bitmap_image& blurredImage, const bitmap_image& unblurredImage, bitmap_image& differenceImage);
};

