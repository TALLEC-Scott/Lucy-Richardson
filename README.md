# Lucy-Richardson deconvolution algorithm in C++

### Description
This is a simple project that demonstrates the Lucy-Richardson deconvolution algorithm. The project is an interface with
which you can apply many different kinds of blurs and noises and then apply the Lucy-Richardson deconvolution algorithm to
restore the original image. The project is written in C++ and uses GTK+3 for the GUI.

### Features
- Apply Blur
    - Gaussian Blur
    - Motion Blur
- Apply Noise
    - Gaussian Noise
    - Salt and Pepper Noise
    - Shot Noise
    - Speckle Noise
- Apply Lucy-Richardson Deconvolution Algorithm
    - Number of iterations
    - Tikhonov regularization or *auto-deconvolution*
- Save Image

### Requirements

GTK+3 libraries are required to build and run the project. The project has been tested on Ubuntu 20.04.

### Build

```bash
cmake .
make
```

### Run

```bash
./lucy
```

### Examples

The original image is shown below.

![Original Image](./resources/airplane.bmp)

The blurred image is shown below.

![Blurred Image](./examples/airplane_blurred.bmp)

The restored image is shown below.

![Restored Image](./examples/airplane_deblurred.bmp)

### Screenshots

The main interface is shown below.

![Main Interface](./screenshots/GUI_screenshot.png)

### Authors
- [Scott TALLEC](https://github.com/TALLEC-Scott)
- [Daniel Rosa]

