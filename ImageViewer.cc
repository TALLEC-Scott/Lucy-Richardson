#include "ImageViewer.hh"
#include "deconvolution.hh"
#include "blur_image.hh"

ImageViewer::ImageViewer() : window(nullptr), image(nullptr), pixbuf(nullptr) {}

void ImageViewer::run(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new("com.example.image_viewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(ImageViewer::activate), this);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
}

void ImageViewer::openImage(GtkWidget* widget, gpointer data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(data);
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open Image", GTK_WINDOW(viewer->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, nullptr);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        viewer->imagePath = filename;
        viewer->bitmapImage = bitmap_image(filename);
        if (viewer->bitmapImage.data()) {
            viewer->pixbuf = gdk_pixbuf_new_from_file(filename, nullptr);
            gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image), viewer->pixbuf);
        } else {
            GtkWidget* errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                            "Failed to open image!");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

unsigned char* ImageViewer::convertToRGBBuffer(const bitmap_image& image) {
    const unsigned int width = image.width();
    const unsigned int height = image.height();
    const unsigned int numChannels = 3;  // RGB

    unsigned char* buffer = new unsigned char[width * height * numChannels];
    unsigned int index = 0;

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            rgb_t color = image.get_pixel(x, y);
            buffer[index++] = color.red;
            buffer[index++] = color.green;
            buffer[index++] = color.blue;
        }
    }

    return buffer;
}

void ImageViewer::deconvolveImage(GtkWidget* widget, gpointer data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(data);
    if (!viewer->bitmapImage.data()) {
        GtkWidget* infoDialog = gtk_message_dialog_new(GTK_WINDOW(viewer->window), GTK_DIALOG_MODAL,
                                                       GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                       "Please open an image first!");
        gtk_dialog_run(GTK_DIALOG(infoDialog));
        gtk_widget_destroy(infoDialog);
        return;
    }

    //int iterations = 50;  // Number of deconvolution iterations
    ImageBlurrer::BlurType blurTypes[3] = {ImageBlurrer::GAUSSIAN, ImageBlurrer::BOX, ImageBlurrer::MOTION};
    ImageBlurrer blurrer(blurTypes[0], 3, 3.0, 45.0);
    blurrer.loadImage("resources/image.bmp");
    blurrer.blurImage();
    blurrer.addNoise(0.0, 10.0, ImageBlurrer::SALT_AND_PEPPER);
    std::string blurredImageFile = "results/blurred_image" + std::to_string(0) + ".bmp";
    std::string diffImageFile = "results/diff_image" + std::to_string(0) + ".bmp";
    blurrer.saveImage(blurredImageFile);


    // Load the blurred image
    bitmap_image blurredImage(blurredImageFile);

    // Get the PSF used to blur the image
    std::vector<std::vector<double>> kernel = blurrer.getKernel();
    // Perform Richardson-Lucy deconvolution
    auto x = Deconvolver(kernel, blurredImage);
    x.deconvolve(3);
    bitmap_image restoredImage = x.image;


    // Create a GdkPixbuf from the restored image
    unsigned char* buffer = convertToRGBBuffer(restoredImage);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8, restoredImage.width(),
                                                 restoredImage.height(), restoredImage.width() *3, nullptr, nullptr);
    viewer->pixbuf = pixbuf;
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image), pixbuf);

    // Clean up memory
    delete[] buffer;
    restoredImage.clear();
}

void ImageViewer::activate(GtkApplication* app, gpointer user_data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(user_data);
    viewer->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(viewer->window), "Image Viewer");
    gtk_window_set_default_size(GTK_WINDOW(viewer->window), 400, 300);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(viewer->window), box);

    viewer->image = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(box), viewer->image, TRUE, TRUE, 0);

    GtkWidget* openButton = gtk_button_new_with_label("Open Image");
    g_signal_connect(openButton, "clicked", G_CALLBACK(ImageViewer::openImage), viewer);
    gtk_box_pack_start(GTK_BOX(box), openButton, FALSE, FALSE, 0);

    GtkWidget* deconvolveButton = gtk_button_new_with_label("Deconvolution");
    g_signal_connect(deconvolveButton, "clicked", G_CALLBACK(ImageViewer::deconvolveImage), viewer);
    gtk_box_pack_start(GTK_BOX(box), deconvolveButton, FALSE, FALSE, 0);

    gtk_widget_show_all(viewer->window);
}
