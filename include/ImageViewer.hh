#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <gtk/gtk.h>
#include <bitmap_image.hpp>
#include "blur_image.hh"

class ImageViewer {
    enum class DeconvolutionType {
        RICHARDSON_LUCY,
        RICHARDSON_LUCY_TV,
        RICHARDSON_LUCY_TIKHONOV,
    };
public:
    ImageViewer();
    void run(int argc, char* argv[]);

private:
    static void openImage(GtkWidget* widget, gpointer data);
    static void deconvolveImage(GtkWidget* widget, gpointer data);
    static void blurTypeChanged(GtkComboBox* comboBox, gpointer data);
    static void activate(GtkApplication* app, gpointer user_data);

    GtkWidget* window;
    GtkWidget* imageOriginal;    // Widget to display the original image
    GtkWidget* imageBlurred;     // Widget to display the blurred image
    GtkWidget* imageDeblurred;   // Widget to display the deblurred image

    GdkPixbuf* pixbufOriginal;   // Pixbuf for the original image
    GdkPixbuf* pixbufBlurred;    // Pixbuf for the blurred image
    GdkPixbuf* pixbufDeblurred;  // Pixbuf for the deblurred image
    std::string imagePath;
    ImageBlurrer::BlurType blurType = ImageBlurrer::GAUSSIAN;
    bitmap_image bitmapImage;
    bitmap_image blurredImage;
    bitmap_image deblurredImage;
    std::vector<std::vector<double>> kernel;
    GtkWidget* noiseComboBox;
    GtkWidget* blurComboBox;
    GtkWidget* deconvolutionComboBox;

    static unsigned char *convertToRGBBuffer(const bitmap_image &image);

    GtkWidget *image;
    ImageBlurrer::NoiseType noiseType = ImageBlurrer::NOISE_NONE;

    DeconvolutionType deconvolutionType = DeconvolutionType::RICHARDSON_LUCY;

    int numberOfIterations = 3;
    bool autoIterations = false;

    static void MenuChanged(GtkComboBox *comboBox, gpointer data);

    static void iterationsChanged(GtkRange *range, gpointer data);

    static void saveImage(GtkWidget *widget, gpointer data);
};

#endif  // IMAGE_VIEWER_H
