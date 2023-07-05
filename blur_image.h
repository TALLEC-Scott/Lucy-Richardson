#include "bitmap_image.hpp"
#include <vector>
#include "bitmap_image.hpp"
#include <iostream>
#include <map>
#include <random>
#include <string>



class ImageBlurrer {
public:
    enum BlurType { GAUSSIAN, BOX, MOTION };
    enum NoiseType { SALT_AND_PEPPER, GAUSS};

    ImageBlurrer(BlurType type, int kernelSize, double sigma = 0.0, double angle = 0.0);

    void loadImage(const std::string& filePath);
    void saveImage(const std::string& filePath);
    void blurImage();
    void addNoise(double mean, double stddev);
    void denoiseImage(int neighborhoodSize);
    void getNeighborhood(std::size_t x, std::size_t y, int size, 
        std::vector<double>& redNeighborhood, std::vector<double>& greenNeighborhood, std::vector<double>& blueNeighborhood);
    std::vector<std::vector<double>> getKernel() { return kernel; }

    void addNoise(double mean, double stddev, NoiseType type);

private:
    int kernelSize;
    bitmap_image image;
    std::vector<std::vector<double>> kernel;

    void createGaussianKernel(double sigma);
    void createBoxBlurKernel(int size);
    void createMotionBlurKernel(int size, double angle);

    void addGaussianNoise(double mean, double stddev);

    void addSaltAndPepperNoise(double mean, double stddev);

};

ImageBlurrer::ImageBlurrer(BlurType type, int kernelSize, double sigma, double angle) : kernelSize(kernelSize) {
    switch (type) {
        case BlurType::GAUSSIAN:
            createGaussianKernel(sigma);
            break;
        case BlurType::BOX:
            createBoxBlurKernel(kernelSize);
            break;
        case BlurType::MOTION:
            createMotionBlurKernel(kernelSize, angle);
            break;
        default:
            throw std::invalid_argument("Invalid blur type.");
    }
}


void ImageBlurrer::loadImage(const std::string& filePath) {
    image = bitmap_image(filePath);
}

void ImageBlurrer::saveImage(const std::string& filePath) {
    image.save_image(filePath);
}

void ImageBlurrer::createGaussianKernel(double sigma) {
    kernel = std::vector<std::vector<double>>(kernelSize, std::vector<double>(kernelSize));
    double sum = 0.0;
    int halfSize = kernelSize / 2;
    for (int x = -halfSize; x <= halfSize; x++) {
        for (int y = -halfSize; y <= halfSize; y++) {
            kernel[x + halfSize][y + halfSize] = exp(-(x * x + y * y) / (2.0 * sigma * sigma)) / (2.0 * M_PI * sigma * sigma);
            sum += kernel[x + halfSize][y + halfSize];
        }
    }
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            kernel[i][j] /= sum;
        }
    }
}

void ImageBlurrer::createBoxBlurKernel(int kernelSize) {
    kernel = std::vector<std::vector<double>>(kernelSize, std::vector<double>(kernelSize, 1.0/(kernelSize*kernelSize)));
}

void ImageBlurrer::createMotionBlurKernel(int kernelSize, double angle) {
    kernel = std::vector<std::vector<double>>(kernelSize, std::vector<double>(kernelSize));
    int center = kernelSize / 2;
    double angleInRadians = angle * M_PI / 180.0;
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            double x = i - center;
            double y = j - center;
            kernel[i][j] = (abs(y - x*tan(angleInRadians)) <= 0.5) ? 1.0 / kernelSize : 0.0;
        }
    }
}


void ImageBlurrer::blurImage() {
    int width = image.width();
    int height = image.height();
    bitmap_image blurredImage(width, height);
    int halfSize = kernelSize / 2;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            double red = 0.0, green = 0.0, blue = 0.0;
            for (int i = -halfSize; i <= halfSize; i++) {
                for (int j = -halfSize; j <= halfSize; j++) {
                    int xi = x + i;
                    int yj = y + j;
                    if (xi >= 0 && xi < width && yj >= 0 && yj < height) {
                        rgb_t color = image.get_pixel(xi, yj);
                        red += color.red * kernel[i + halfSize][j + halfSize];
                        green += color.green * kernel[i + halfSize][j + halfSize];
                        blue += color.blue * kernel[i + halfSize][j + halfSize];
                    }
                }
            }
            rgb_t newColor;
            newColor.red = static_cast<unsigned char>(std::min(std::max(int(red), 0), 255));
            newColor.green = static_cast<unsigned char>(std::min(std::max(int(green), 0), 255));
            newColor.blue = static_cast<unsigned char>(std::min(std::max(int(blue), 0), 255));
            blurredImage.set_pixel(x, y, newColor);
        }
    }

    image = blurredImage;
}

void ImageBlurrer::addGaussianNoise(double mean, double stddev) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{mean, stddev};

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color = image.get_pixel(x, y);
            color.red = std::min(255.0, std::max(0.0, color.red + d(gen)));
            color.green = std::min(255.0, std::max(0.0, color.green + d(gen)));
            color.blue = std::min(255.0, std::max(0.0, color.blue + d(gen)));
            image.set_pixel(x, y, color);
        }
    }
}

<<<<<<< HEAD
void ImageBlurrer::getNeighborhood(std::size_t x, std::size_t y, int size, 
        std::vector<double>& redNeighborhood, std::vector<double>& greenNeighborhood, std::vector<double>& blueNeighborhood) {
    int halfSize = size / 2;
    for (int i = -halfSize; i <= halfSize; i++) {
        for (int j = -halfSize; j <= halfSize; j++) {
            if (x+i >= 0 && x+i < image.width() && y+j >= 0 && y+j < image.height()) {
                rgb_t color = image.get_pixel(x+i, y+j);
                redNeighborhood.push_back(color.red);
                greenNeighborhood.push_back(color.green);
                blueNeighborhood.push_back(color.blue);
            }
        }
    }
}

void ImageBlurrer::denoiseImage(int neighborhoodSize) {
    bitmap_image denoisedImage(image.width(), image.height());

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            std::vector<double> redNeighborhood, greenNeighborhood, blueNeighborhood;
            getNeighborhood(x, y, neighborhoodSize, redNeighborhood, greenNeighborhood, blueNeighborhood);
            
            std::sort(redNeighborhood.begin(), redNeighborhood.end());
            std::sort(greenNeighborhood.begin(), greenNeighborhood.end());
            std::sort(blueNeighborhood.begin(), blueNeighborhood.end());

            rgb_t color;
            color.red = redNeighborhood[redNeighborhood.size() / 2];
            color.green = greenNeighborhood[greenNeighborhood.size() / 2];
            color.blue = blueNeighborhood[blueNeighborhood.size() / 2];
            denoisedImage.set_pixel(x, y, color);
        }
    }

    image = denoisedImage;
}
=======
void ImageBlurrer::addSaltAndPepperNoise(double mean, double stddev) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{mean, stddev};

    for (std::size_t x = 0; x < image.width(); x++) {
        for (std::size_t y = 0; y < image.height(); y++) {
            rgb_t color = image.get_pixel(x, y);
            color.red = std::min(255.0, std::max(0.0, color.red + d(gen)));
            color.green = std::min(255.0, std::max(0.0, color.green + d(gen)));
            color.blue = std::min(255.0, std::max(0.0, color.blue + d(gen)));
            image.set_pixel(x, y, color);
        }
    }

}

#include <random>

void ImageBlurrer::addNoise(double mean, double stddev, NoiseType type) {

    switch (type) {
        case NoiseType::SALT_AND_PEPPER:
            addSaltAndPepperNoise(mean, stddev);
            break;
        case NoiseType::GAUSS:
            addGaussianNoise(mean, stddev);
        default:
            throw std::invalid_argument("Invalid noise type.");
    }
}

>>>>>>> 58aab6bbf77b05e2ba4b485af2914c85df6c0b84
