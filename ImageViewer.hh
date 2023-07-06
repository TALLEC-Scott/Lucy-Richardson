#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <gtk/gtk.h>
#include <bitmap_image.hpp>
#include "blur_image.hh"

class ImageViewer {
public:
    ImageViewer();
    void run(int argc, char* argv[]);

private:
    static void openImage(GtkWidget* widget, gpointer data);
    static void deconvolveImage(GtkWidget* widget, gpointer data);
    static void blurTypeChanged(GtkComboBox* comboBox, gpointer data);
    static void activate(GtkApplication* app, gpointer user_data);

    GtkWidget* window;
    GtkWidget* image1;
    GtkWidget* image2;
    GdkPixbuf* pixbuf1;
    GdkPixbuf* pixbuf2;
    std::string imagePath;
    ImageBlurrer::BlurType blurType;
    bitmap_image bitmapImage;
    std::vector<std::vector<double>> kernel;


    static unsigned char *convertToRGBBuffer(const bitmap_image &image);

};

#endif  // IMAGE_VIEWER_H
