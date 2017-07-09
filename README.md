# CPP_ImageProcessing_TBB
Project I worked on as a part of my University Degree (Computer Games Development). Final grade for this coursework: 70%

Was required to apply techniquese discussed in class to implement image processing operations usibg Threading Building Blocks (TBB). Two parts to this task, both written in a single file.

1. Used the stencil pattern to implement a TBB kernel to blur an image. Needed to be written in both a sequential AND a parallel approach.

2. Use supplied image loading code and update the splitting of the  RGB values from a sequential approach to a parallel approach. Aim was to take two similar images, and output a single image that used absolute differences in RGB values to determine the differences between the two images. A binary threshold was then used to make the image black and white.

This coursework used the Linux OS to complete. By default, this repo has enabled both parallel + sequential methods, and outputs the timings of both methods to the console. There are also 2 Kernels available to use (7x7 and a 5x5) and by default uses the 7x7 kernel (changable by changing "origMatrixSize")
