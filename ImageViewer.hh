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
    static void activate(GtkApplication* app, gpointer user_data);

    GtkWidget* window;
    GtkWidget* image;
    GdkPixbuf* pixbuf;
    std::string imagePath;
    bitmap_image bitmapImage;
    std::vector<std::vector<double>> kernel;


    static unsigned char *convertToRGBBuffer(const bitmap_image &image);

    ImageBlurrer::BlurType blurType;
};

#endif  // IMAGE_VIEWER_H
