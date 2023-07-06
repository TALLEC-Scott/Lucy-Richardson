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


