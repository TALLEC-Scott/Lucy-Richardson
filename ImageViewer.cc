#include "ImageViewer.hh"
#include "deconvolution.hh"
#include "blur_image.hh"

ImageViewer::ImageViewer() : window(nullptr), imageOriginal(nullptr), imageBlurred(nullptr), imageDeblurred(nullptr),
pixbufOriginal(nullptr), pixbufDeblurred(nullptr), pixbufBlurred(nullptr) {}

void ImageViewer::run(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new("com.example.image_viewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(ImageViewer::activate), this);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
}

void ImageViewer::openImage(GtkWidget* widget, gpointer data) {
    ImageViewer *viewer = static_cast<ImageViewer *>(data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Image", GTK_WINDOW(viewer->window),
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, nullptr);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        viewer->bitmapImage = bitmap_image(filename);
        if (viewer->bitmapImage.data()) {
// Create a GdkPixbuf from the image
            unsigned char *buffer = convertToRGBBuffer(viewer->bitmapImage);
            GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8,
                                                         viewer->bitmapImage.width(),
                                                         viewer->bitmapImage.height(), viewer->bitmapImage.width() * 3,
                                                         nullptr, nullptr);
            viewer->pixbufOriginal = pixbuf;
            gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->imageOriginal), viewer->pixbufOriginal);
            //delete[] buffer;
        } else {
            GtkWidget *errorDialog = gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                            "Failed to open image!");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
    MenuChanged(nullptr, viewer);

}
void ImageViewer::MenuChanged(GtkComboBox* comboBox, gpointer data) {

    ImageViewer* viewer = static_cast<ImageViewer*>(data);

    gint active = gtk_combo_box_get_active(comboBox);
    if (comboBox == GTK_COMBO_BOX(viewer->blurComboBox)) {
            if (active == 0) {
                viewer->blurType = ImageBlurrer::GAUSSIAN;
            } else if (active == 1) {
                viewer->blurType = ImageBlurrer::BOX;
            } else if (active == 2) {
                viewer->blurType = ImageBlurrer::MOTION;
            }
            else if (active == 3)
            {
                viewer->blurType = ImageBlurrer::BLUR_NONE;
            }
            // Handle blur combo box
        } else if (comboBox == GTK_COMBO_BOX(viewer->noiseComboBox)) {
        if (active == 0)
            viewer->noiseType = ImageBlurrer::NOISE_NONE;
        else if (active == 1)
            viewer->noiseType = ImageBlurrer::GAUSS;
        else if (active == 2)
            viewer->noiseType = ImageBlurrer::SALT_AND_PEPPER;
        else if (active == 3)
            viewer->noiseType = ImageBlurrer::POISSON;
        else if (active == 4)
            viewer->noiseType = ImageBlurrer::SPECKLE;
            // Handle noise combo box
        } else if (comboBox == GTK_COMBO_BOX(viewer->deconvolutionComboBox)) {
        if (active == 0)
            viewer->deconvolutionType = DeconvolutionType::RICHARDSON_LUCY;
        else if (active == 1)
            viewer->deconvolutionType = DeconvolutionType::RICHARDSON_LUCY_TIKHONOV;
        else if (active == 2)
            viewer->deconvolutionType = DeconvolutionType::RICHARDSON_LUCY_TV;
    }
    if (viewer->pixbufOriginal == nullptr)
        return;

    // Perform the blur operation with the selected blur type and update the image
    // Here, you can use the viewer->blurType to perform the blur operation or pass it to the ImageBlurrer class
    // After blurring, update the image2 and pixbuf2 with the blurred image data
    //
    ImageBlurrer blurrer(viewer->blurType, 7, 3.0, 90);
    blurrer.loadImage(viewer->bitmapImage);
    blurrer.addNoise(0.0, 10.0, viewer->noiseType);

    blurrer.blurImage();
    char* blurredImageFile = "results/blurred_image.bmp";

    blurrer.saveImage(blurredImageFile);
    viewer->kernel = blurrer.getKernel();


    viewer->pixbufBlurred = gdk_pixbuf_new_from_file(blurredImageFile, nullptr);
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->imageBlurred), viewer->pixbufBlurred);

    auto deconvolver = Deconvolver(viewer->kernel, blurredImageFile);
    switch (viewer->deconvolutionType)
    {
        case DeconvolutionType::RICHARDSON_LUCY:
            deconvolver.deconvolve(viewer->numberOfIterations);
            break;
        case DeconvolutionType::RICHARDSON_LUCY_TIKHONOV:
            deconvolver.deconvolveAuto(viewer->numberOfIterations, 0.1);
            break;
        case DeconvolutionType::RICHARDSON_LUCY_TV:
            deconvolver.deconvolveTV(viewer->numberOfIterations, 0.1, 0.01, 1);
            break;
    }
    viewer->deblurredImage = deconvolver.image;

    // Create a GdkPixbuf from the restored image
    unsigned char* buffer = convertToRGBBuffer(viewer->deblurredImage);
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8, viewer->deblurredImage.width(),
                                                 viewer->deblurredImage.height(), viewer->deblurredImage.width() *3, nullptr, nullptr);
    viewer->pixbufDeblurred = pixbuf;
    gtk_image_set_from_pixbuf(GTK_IMAGE(viewer->imageDeblurred), viewer->pixbufDeblurred);
    //delete[] buffer;
}

void ImageViewer::iterationsChanged(GtkRange* range, gpointer data) {


    ImageViewer* viewer = static_cast<ImageViewer*>(data);

    double value = gtk_range_get_value(range);
    viewer->numberOfIterations = static_cast<int>(value);
    //check if images are loaded
    if (viewer->pixbufOriginal == nullptr)
        return;
    g_idle_add([](gpointer data) {
        ImageViewer* viewer = static_cast<ImageViewer*>(data);
        // call the callback function from menu change
        MenuChanged(GTK_COMBO_BOX(viewer->blurComboBox), data);

        return G_SOURCE_REMOVE;
    }, viewer);
}

void ImageViewer::saveImage(GtkWidget* widget, gpointer data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(data);
    // Perform the image saving logic here

    // For example, you can use GTK file chooser dialog to prompt the user for the save location
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save Image", GTK_WINDOW(viewer->window), GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        //save the image
        viewer->deblurredImage.save_image(filename);
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

void ImageViewer::activate(GtkApplication* app, gpointer user_data) {
    ImageViewer* viewer = static_cast<ImageViewer*>(user_data);
    viewer->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(viewer->window), "Richardson-Lucy Deconvolution");
    gtk_window_set_default_size(GTK_WINDOW(viewer->window), 800, 450);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(viewer->window), box);

    GtkWidget* imageBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); // Use horizontal box here
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_pack_start(GTK_BOX(box), imageBox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), controlBox, FALSE, FALSE, 0);

    viewer->imageOriginal = gtk_image_new();
    viewer->imageBlurred = gtk_image_new();
    viewer->imageDeblurred = gtk_image_new();
    gtk_box_pack_start(GTK_BOX(imageBox), viewer->imageOriginal, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(imageBox), viewer->imageBlurred, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(imageBox), viewer->imageDeblurred, TRUE, TRUE, 0);

    GtkWidget* openButton = gtk_button_new_with_label("Open Image");
    g_signal_connect(openButton, "clicked", G_CALLBACK(ImageViewer::openImage), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), openButton, FALSE, FALSE, 0);
    // Deconvolution menu
    GtkWidget* deconvolutionLabel = gtk_label_new("Select Deconvolution Method:");
    gtk_box_pack_start(GTK_BOX(controlBox), deconvolutionLabel, FALSE, FALSE, 0);

    GtkWidget* deconvolutionComboBox = gtk_combo_box_text_new();
    viewer->deconvolutionComboBox = deconvolutionComboBox;
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(deconvolutionComboBox), "Richardson-Lucy");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(deconvolutionComboBox), "Richardson-Lucy with Tikhonov Regularization");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(deconvolutionComboBox), "Richardson-Lucy with TV Regularization");
    gtk_combo_box_set_active(GTK_COMBO_BOX(deconvolutionComboBox), 0);
    g_signal_connect(deconvolutionComboBox, "changed", G_CALLBACK(ImageViewer::MenuChanged), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), deconvolutionComboBox, FALSE, FALSE, 0);

    //Blur menu
    GtkWidget* blurLabel = gtk_label_new("Select Blur Type:");
    gtk_box_pack_start(GTK_BOX(controlBox), blurLabel, FALSE, FALSE, 0);

    GtkWidget* blurComboBox = gtk_combo_box_text_new();
    viewer->blurComboBox = blurComboBox;
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Gaussian");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Box");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "Motion");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(blurComboBox), "None");

    gtk_combo_box_set_active(GTK_COMBO_BOX(blurComboBox), 0);
    g_signal_connect(blurComboBox, "changed", G_CALLBACK(ImageViewer::MenuChanged), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), blurComboBox, FALSE, FALSE, 0);

    GtkWidget* noiseLabel = gtk_label_new("Select Noise Type:");
    gtk_box_pack_start(GTK_BOX(controlBox), noiseLabel, FALSE, FALSE, 0);

    GtkWidget* noiseComboBox = gtk_combo_box_text_new();
    viewer->noiseComboBox = noiseComboBox;
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(noiseComboBox), "None");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(noiseComboBox), "Gaussian");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(noiseComboBox), "Salt and Pepper");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(noiseComboBox), "Poisson");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(noiseComboBox), "Speckle");

    gtk_combo_box_set_active(GTK_COMBO_BOX(noiseComboBox), 0);
    g_signal_connect(noiseComboBox, "changed", G_CALLBACK(ImageViewer::MenuChanged), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), noiseComboBox, FALSE, FALSE, 0);


    // Iterations slider
    GtkWidget* sliderLabel = gtk_label_new("Number of Iterations");
    gtk_box_pack_start(GTK_BOX(controlBox), sliderLabel, FALSE, FALSE, 0);

    GtkWidget* iterationsSlider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 10, 1);
    gtk_range_set_value(GTK_RANGE(iterationsSlider), 1);
    g_signal_connect(iterationsSlider, "value-changed", G_CALLBACK(ImageViewer::iterationsChanged), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), iterationsSlider, FALSE, FALSE, 0);

    // Save button
    GtkWidget* saveButton = gtk_button_new_with_label("Save Image");
    g_signal_connect(saveButton, "clicked", G_CALLBACK(ImageViewer::saveImage), viewer);
    gtk_box_pack_start(GTK_BOX(controlBox), saveButton, FALSE, FALSE, 0);



    gtk_widget_show_all(viewer->window);
}
