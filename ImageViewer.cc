#include "ImageViewer.hh"
#include "deconvolution.hh"
#include "blur_image.hh"

ImageViewer::ImageViewer() : window(nullptr), image1(nullptr), image2(nullptr), pixbuf1(nullptr), pixbuf2(nullptr) {}

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
        viewer->bitmapImage = bitmap_image(filename);
        if (viewer->bitmapImage.data()) {
            GtkWidget* dialogBox = gtk_dialog_new_with_buttons("Select Blur Type", GTK_WINDOW(viewer->window),
                                                               GTK_DIALOG_MODAL,
                                                               "_Cancel", GTK_RESPONSE_CANCEL,
                                                               "_OK", GTK_RESPONSE_OK, nullptr);
            GtkWidget* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialogBox));

            GtkWidget* label = gtk_label_new("Select Blur Type:");
            gtk_box_pack_start(GTK_BOX(contentArea), label, FALSE, FALSE, 0);

            GtkWidget* comboBox = gtk_combo_box_text_new();
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "Gaussian");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "Box");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboBox), "Motion");
            gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), 0);
            gtk_box_pack_start(GTK_BOX(contentArea), comboBox, FALSE, FALSE, 0);

            gtk_widget_show_all(dialogBox);
            if (gtk_dialog_run(GTK_DIALOG(dialogBox)) == GTK_RESPONSE_OK) {
                gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(comboBox));
                if (active == 0) {
                    viewer->blurType = ImageBlurrer::GAUSSIAN;
                } else if (active == 1) {
                    viewer->blurType = ImageBlurrer::BOX;
                } else if (active == 2) {
                    viewer->blurType = ImageBlurrer::MOTION;
                }
            }
            gtk_widget_destroy(dialogBox);

            //int iterations = 50;  // Number of deconvolution iterations
            ImageBlurrer blurrer(viewer->blurType, 3, 3.0, 45.0);
            blurrer.loadImage(filename);
            blurrer.blurImage();
            blurrer.addNoise(0.0, 10.0, ImageBlurrer::SALT_AND_PEPPER);
            char* blurredImageFile = "results/blurred_image.bmp";
            blurrer.saveImage(blurredImageFile);
            viewer->kernel = blurrer.getKernel();
            viewer->pixbuf1 = gdk_pixbuf_new_from_file(blurredImageFile, nullptr);
            gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image1), viewer->pixbuf1);
        } else {
            GtkWidget* errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                            "Failed to open image!");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }
       // g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void ImageViewer::blurTypeChanged(GtkComboBox* comboBox, gpointer data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(data);
    gint active = gtk_combo_box_get_active(comboBox);
    if (active == 0) {
        viewer->blurType = ImageBlurrer::GAUSSIAN;
    } else if (active == 1) {
        viewer->blurType = ImageBlurrer::BOX;
    } else if (active == 2) {
        viewer->blurType = ImageBlurrer::MOTION;
    }

    // Perform the blur operation with the selected blur type and update the image
    // Here, you can use the viewer->blurType to perform the blur operation or pass it to the ImageBlurrer class
    // After blurring, update the image2 and pixbuf2 with the blurred image data
    //}
    ImageBlurrer blurrer(viewer->blurType, 3, 3.0, 45.0);
    blurrer.loadImage(viewer->bitmapImage);

    blurrer.blurImage();
    blurrer.addNoise(0.0, 10.0, ImageBlurrer::SALT_AND_PEPPER);
    char* blurredImageFile = "results/blurred_image.bmp";

    blurrer.saveImage(blurredImageFile);
    viewer->kernel = blurrer.getKernel();


    viewer->pixbuf1 = gdk_pixbuf_new_from_file(blurredImageFile, nullptr);
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image1), viewer->pixbuf1);

    auto deconvolver = Deconvolver(viewer->kernel, blurredImageFile);
    deconvolver.deconvolve(3);
    bitmap_image restoredImage = deconvolver.image;

    // Create a GdkPixbuf from the restored image
    unsigned char* buffer = convertToRGBBuffer(restoredImage);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8, restoredImage.width(),
                                                 restoredImage.height(), restoredImage.width() *3, nullptr, nullptr);
    viewer->pixbuf2 = pixbuf;
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image2), viewer->pixbuf2);
    delete[] buffer;
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


    auto x = Deconvolver(viewer->kernel, viewer->bitmapImage);
    x.deconvolve(3);
    bitmap_image restoredImage = x.image;


    // Create a GdkPixbuf from the restored image
    unsigned char* buffer = convertToRGBBuffer(restoredImage);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8, restoredImage.width(),
                                                 restoredImage.height(), restoredImage.width() *3, nullptr, nullptr);
    viewer->pixbuf2 = pixbuf;
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->image2), pixbuf);

    // Clean up memory
    delete[] buffer;
    restoredImage.clear();
}

void ImageViewer::activate(GtkApplication* app, gpointer user_data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(user_data);
    viewer->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(viewer->window), "Image Viewer");
    gtk_window_set_default_size(GTK_WINDOW(viewer->window), 800, 400);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(viewer->window), box);

    GtkWidget* imageBox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* imageBox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box), imageBox1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), imageBox2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), controlBox, FALSE, FALSE, 0);

    viewer->image1 = gtk_image_new();
    viewer->image2 = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(imageBox1), viewer->image1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(imageBox2), viewer->image2, TRUE, TRUE, 0);

    GtkWidget* openButton = gtk_button_new_with_label("Open Image");
    g_signal_connect(openButton, "clicked", G_CALLBACK(ImageViewer::openImage), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), openButton, FALSE, FALSE, 0);

    GtkWidget* blurComboBox = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Gaussian");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Box");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Motion");
    gtk_combo_box_set_active(GTK_COMBO_BOX(blurComboBox), 0);
    g_signal_connect(blurComboBox, "changed", G_CALLBACK(ImageViewer::blurTypeChanged), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), blurComboBox, FALSE, FALSE, 0);

    gtk_widget_show_all(viewer->window);
}
