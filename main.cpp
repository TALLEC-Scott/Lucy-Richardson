#include "blur_image.hh"
#include "deconvolution.hh"
#include "DeconvolutionUtils.hh"
#include <cmath>
#include <limits>
#include <iostream>

#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>
#include <bitmap_image.hpp>
#include "deconvolution.hh"
#include "blur_image.hh"

typedef struct {
    GtkWidget *window;
    GtkWidget *imageLabel;
    GtkWidget *kernelLabel;
    GtkWidget *imageButton;
    GtkWidget *kernelButton;
    GtkWidget *deconvolveButton;
    GtkWidget *iterationsSpinButton;
    bitmap_image image;
    std::vector<std::vector<double>> kernel;
} AppData;

static void open_image(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Image", GTK_WINDOW(app_data->window), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        app_data->image = bitmap_image(filename);
        if (app_data->image.width() > 0 && app_data->image.height() > 0) {
            gtk_label_set_text(GTK_LABEL(app_data->imageLabel), filename);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->window), GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                             "Failed to open image!");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void open_kernel(GtkWidget *widget, gpointer data, int i) {
    ImageBlurrer::BlurType blurTypes[3] = {ImageBlurrer::GAUSSIAN, ImageBlurrer::BOX, ImageBlurrer::MOTION};
    ImageBlurrer blurrer(blurTypes[i], 3, 3.0, 45.0);
    blurrer.loadImage("resources/image.bmp");
    blurrer.blurImage();
    blurrer.addNoise(0.0, 10.0, ImageBlurrer::SALT_AND_PEPPER);
    std::string blurredImageFile = "results/blurred_image" + std::to_string(i) + ".bmp";
    std::string diffImageFile = "results/diff_image" + std::to_string(i) + ".bmp";
    blurrer.saveImage(blurredImageFile);


    // Load the blurred image
    bitmap_image blurredImage(blurredImageFile);

    // Get the PSF used to blur the image
    std::vector<std::vector<double>> kernel = blurrer.getKernel();
    AppData *app_data = (AppData *)data;
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Kernel", GTK_WINDOW(app_data->window), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        app_data->kernel = kernel;
        if (app_data->kernel.size() > 0 && app_data->kernel[0].size() > 0) {
            gtk_label_set_text(GTK_LABEL(app_data->kernelLabel), filename);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->window), GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                             "Failed to open kernel!");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void deconvolve(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *)data;
    gint iterations = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_data->iterationsSpinButton));

    if (!app_data->image) {
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->window), GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                        "Please select an image and a kernel!");
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
        return;
    }

    // Perform Richardson-Lucy deconvolution
    auto x = Deconvolver(app_data->kernel, app_data->image);
    x.deconvolve(3);
    bitmap_image restored_image = x.image;

    // Save the restored image
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Restored Image", GTK_WINDOW(app_data->window), GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        restored_image.save_image(filename);
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->window), GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                        "Deconvolution completed and restored image saved successfully!");
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
        g_free(filename);
    } else {
        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(app_data->window), GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                        "Deconvolution completed!");
        gtk_dialog_run(GTK_DIALOG(info_dialog));
        gtk_widget_destroy(info_dialog);
    }
    gtk_widget_destroy(dialog);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *iterationsLabel;
    GtkWidget *iterationsAdjustment;
    GtkWidget *imageButton;
    GtkWidget *kernelButton;
    GtkWidget *deconvolveButton;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Richardson-Lucy Deconvolution");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    iterationsLabel = gtk_label_new("Iterations:");
    gtk_grid_attach(GTK_GRID(grid), iterationsLabel, 0, 0, 1, 1);

    iterationsAdjustment = reinterpret_cast<GtkWidget *>(gtk_adjustment_new(50.0, 1.0, 100.0, 1.0, 1.0, 0.0));
    GtkWidget *iterationsSpinButton = gtk_spin_button_new(GTK_ADJUSTMENT(iterationsAdjustment), 1.0, 0);
    gtk_grid_attach(GTK_GRID(grid), iterationsSpinButton, 1, 0, 1, 1);
    g_object_set(iterationsSpinButton, "halign", GTK_ALIGN_START, NULL);

    imageButton = gtk_button_new_with_label("Open Image");
    gtk_grid_attach(GTK_GRID(grid), imageButton, 0, 1, 1, 1);
    g_signal_connect(imageButton, "clicked", G_CALLBACK(open_image), user_data);

    GtkWidget *imageLabel = gtk_label_new("No Image Selected");
    gtk_grid_attach(GTK_GRID(grid), imageLabel, 1, 1, 1, 1);
    g_object_set(imageLabel, "halign", GTK_ALIGN_START, NULL);

    kernelButton = gtk_button_new_with_label("Open Kernel");
    gtk_grid_attach(GTK_GRID(grid), kernelButton, 0, 2, 1, 1);
    g_signal_connect(kernelButton, "clicked", G_CALLBACK(open_kernel), user_data);

    GtkWidget *kernelLabel = gtk_label_new("No Kernel Selected");
    gtk_grid_attach(GTK_GRID(grid), kernelLabel, 1, 2, 1, 1);
    g_object_set(kernelLabel, "halign", GTK_ALIGN_START, NULL);

    deconvolveButton = gtk_button_new_with_label("Deconvolve");
    gtk_grid_attach(GTK_GRID(grid), deconvolveButton, 0, 3, 2, 1);
    g_signal_connect(deconvolveButton, "clicked", G_CALLBACK(deconvolve), user_data);

    gtk_widget_show_all(window);
}

#include "ImageViewer.hh"

int main(int argc, char* argv[]) {
    ImageViewer viewer;
    viewer.run(argc, argv);
    return 0;
}
