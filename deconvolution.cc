#include "deconvolution.hh"
#include "DeconvolutionUtils.hh"


Deconvolver::Deconvolver(const std::vector<std::vector<double>>& kernel) : kernel(kernel) {}

void Deconvolver::loadImage(const std::string& filePath) {
    image = bitmap_image(filePath);
}


void Deconvolver::saveImage(const std::string& filePath) {
    image.save_image(filePath);
}

std::vector<std::vector<double>> Deconvolver::convolve(const std::vector<std::vector<double>>& image, const std::vector<std::vector<double>>& kernel) {
    // Create a new image filled with zeros
    std::vector<std::vector<double>> newImage(image.size(), std::vector<double>(image[0].size(), 0));

    // For each pixel of the image
    for (std::size_t x = 0; x < image.size(); x++) {
        for (std::size_t y = 0; y < image[x].size(); y++) {
            // For each value of the kernel
            for (std::size_t i = 0; i < kernel.size(); i++) {
                for (std::size_t j = 0; j < kernel[i].size(); j++) {
                    // Compute the corresponding position in the image
                    int xi = x - i / 2;
                    int yj = y - j / 2;
                    // Check if the position is inside the image
                    if (xi >= 0 && xi < image.size() && yj >= 0 && yj < image[xi].size()) {
                        newImage[x][y] += image[xi][yj] * kernel[i][j];
                    }
                }
            }
        }
    }

    return newImage;
}

void Deconvolver::deconvolve(int iterations) {
    std::vector<std::vector<double>> redImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> greenImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> blueImage(image.width(), std::vector<double>(image.height()));

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color = image.get_pixel(x, y);
            redImage[x][y] = color.red;
            greenImage[x][y] = color.green;
            blueImage[x][y] = color.blue;
        }
    }

    // Perform the deconvolution for each color channel separately
    std::vector<std::vector<double>> colorImages[3] = {redImage, greenImage, blueImage};
    for (int color = 0; color < 3; color++) {
        for (int iter = 0; iter < iterations; iter++) {
            std::vector<std::vector<double>> ratio(image.width(), std::vector<double>(image.height()));
            std::vector<std::vector<double>> convolvedImage = convolve(colorImages[color], kernel);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    ratio[x][y] = colorImages[color][x][y] / convolvedImage[x][y];
                }
            }

            std::vector<std::vector<double>> flippedKernel(kernel.rbegin(), kernel.rend());
            for (auto& row : flippedKernel) {
                std::reverse(row.begin(), row.end());
            }
            std::vector<std::vector<double>> convolvedRatio = convolve(ratio, flippedKernel);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    colorImages[color][x][y] *= convolvedRatio[x][y];
                }
            }
        }
    }

    // Assign the results back to each color channel
    redImage = colorImages[0];
    greenImage = colorImages[1];
    blueImage = colorImages[2];

    // Convert the color images back to an RGB image
    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color;
            color.red = std::min(255.0, std::max(0.0, redImage[x][y]));
            color.green = std::min(255.0, std::max(0.0, greenImage[x][y]));
            color.blue = std::min(255.0, std::max(0.0, blueImage[x][y]));
            image.set_pixel(x, y, color);
        }
    }
}

void Deconvolver::deconvolveAuto(int iterations, double lambda) {
    std::vector<std::vector<double>> redImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> greenImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> blueImage(image.width(), std::vector<double>(image.height()));

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color = image.get_pixel(x, y);
            redImage[x][y] = color.red;
            greenImage[x][y] = color.green;
            blueImage[x][y] = color.blue;
        }
    }

    // Perform the deconvolution for each color channel separately
    std::vector<std::vector<double>> colorImages[3] = {redImage, greenImage, blueImage};
    for (int color = 0; color < 3; color++) {
        for (int iter = 0; iter < iterations; iter++) {
            std::vector<std::vector<double>> ratio(image.width(), std::vector<double>(image.height()));
            std::vector<std::vector<double>> convolvedImage = convolve(colorImages[color], kernel);

            // Laplacian filter for calculating image roughness
            std::vector<std::vector<double>> laplacianFilter = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
            std::vector<std::vector<double>> laplacianImage = convolve(colorImages[color], laplacianFilter);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    ratio[x][y] = colorImages[color][x][y] / (convolvedImage[x][y] + lambda * laplacianImage[x][y]);
                }
            }

            std::vector<std::vector<double>> flippedKernel(kernel.rbegin(), kernel.rend());
            for (auto& row : flippedKernel) {
                std::reverse(row.begin(), row.end());
            }
            std::vector<std::vector<double>> convolvedRatio = convolve(ratio, flippedKernel);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    colorImages[color][x][y] *= convolvedRatio[x][y];
                }
            }
        }
    }
// Assign the results back to each color channel
    redImage = colorImages[0];
    greenImage = colorImages[1];
    blueImage = colorImages[2];

    // Convert the color images back to an RGB image
    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color;
            color.red = std::min(255.0, std::max(0.0, redImage[x][y]));
            color.green = std::min(255.0, std::max(0.0, greenImage[x][y]));
            color.blue = std::min(255.0, std::max(0.0, blueImage[x][y]));
            image.set_pixel(x, y, color);
        }
    }
}

void Deconvolver::deconvolveTV(int iterations, double lambda, double alpha, double scalingFactor) {
    std::vector<std::vector<double>> redImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> greenImage(image.width(), std::vector<double>(image.height()));
    std::vector<std::vector<double>> blueImage(image.width(), std::vector<double>(image.height()));

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color = image.get_pixel(x, y);
            redImage[x][y] = color.red;
            greenImage[x][y] = color.green;
            blueImage[x][y] = color.blue;
        }
    }

    // Perform the deconvolution for each color channel separately
    std::vector<std::vector<double>> colorImages[3] = {redImage, greenImage, blueImage};
    for (int color = 0; color < 3; color++) {
        for (int iter = 0; iter < iterations; iter++) {
            std::vector<std::vector<double>> ratio(image.width(), std::vector<double>(image.height()));
            std::vector<std::vector<double>> convolvedImage = convolve(colorImages[color], kernel);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    ratio[x][y] = colorImages[color][x][y] / convolvedImage[x][y];
                }
            }

            std::vector<std::vector<double>> flippedKernel(kernel.rbegin(), kernel.rend());
            for (auto& row : flippedKernel) {
                std::reverse(row.begin(), row.end());
            }
            std::vector<std::vector<double>> convolvedRatio = convolve(ratio, flippedKernel);

            // TV Regularization
            std::vector<std::vector<double>> difference(image.width(), std::vector<double>(image.height()));
            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    double diff = colorImages[color][x][y] - convolvedRatio[x][y];
                    difference[x][y] = diff;
                }
            }

            std::vector<std::vector<double>> gradientX = DeconvolutionUtils::computeGradientX(colorImages[color]);
            std::vector<std::vector<double>> gradientY = DeconvolutionUtils::computeGradientY(colorImages[color]);

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    double gx = gradientX[x][y];
                    double gy = gradientY[x][y];
                    double tvWeight = alpha / (std::sqrt(gx * gx + gy * gy) + lambda);
                    convolvedRatio[x][y] += tvWeight * difference[x][y];
                }
            }

            for (std::size_t x = 0; x < image.width(); x++) {
                for (std::size_t y = 0; y < image.height(); y++) {
                    colorImages[color][x][y] *= convolvedRatio[x][y];
                }
            }
        }
    }

    // Assign the results back to each color channel
    redImage = colorImages[0];
    greenImage = colorImages[1];
    blueImage = colorImages[2];


    // Convert the color images back to an RGB image
    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color;
            color.red = static_cast<unsigned char>(std::min(255.0, std::max(0.0, redImage[x][y] * scalingFactor)));
            color.green = static_cast<unsigned char>(std::min(255.0, std::max(0.0, greenImage[x][y] * scalingFactor)));
            color.blue = static_cast<unsigned char>(std::min(255.0, std::max(0.0, blueImage[x][y] * scalingFactor)));
            image.set_pixel(x, y, color);
        }
    }
}


Deconvolver::Deconvolver(const std::vector<std::vector<double>> &kernel, bitmap_image image) {
    this->kernel = kernel;
    this->image = image;
}

Deconvolver::Deconvolver(const std::vector<std::vector<double>> &kernel, const std::string &filePath) {
    this->kernel = kernel;
    loadImage(filePath);
}
