#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <gtk/gtk.h>
#include <bitmap_image.hpp>

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

    static unsigned char *convertToRGBBuffer(const bitmap_image &image);
};

#endif  // IMAGE_VIEWER_H
