#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <math.h>
#include <cstdlib>
//Free Image library
#include <FreeImagePlus.h>
#include <chrono>
#include <functional>

using namespace std;
using namespace tbb;
using namespace std::chrono;

double sqr(float val)
{
    return val * val;
}

double gaussian2D(float x, float y, float sigma)
{
    return 1.0f / (2.0f*M_PI*sqr(sigma)) * exp(-((sqr(x) + sqr(y)) / (2.0f*sqr(sigma))));
}

void GaussianBlur()
{
    float matrix5[5][5] =
            {
                    0.003765,	0.015019,	0.023792,	0.015019,	0.003765,
                    0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
                    0.023792,	0.094907,	0.150342,	0.094907,	0.023792,
                    0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
                    0.003765,	0.015019,	0.023792,	0.015019,	0.003765
            };

    float matrix7[7][7] =
            {
                    0.000036,	0.000363,	0.001446,	0.002291,	0.001446,	0.000363,	0.000036,
                    0.000363,	0.003676,	0.014662,	0.023226,	0.014662,	0.003676,	0.000363,
                    0.001446,	0.014662,	0.058488,	0.092651,	0.058488,	0.014662,	0.001446,
                    0.002291,	0.023226,	0.092651,	0.146768,	0.092651,	0.023226,	0.002291,
                    0.001446,	0.014662,	0.058488,	0.092651,	0.058488,	0.014662,	0.001446,
                    0.000363,	0.003676,	0.014662,	0.023226,	0.014662,	0.003676,	0.000363,
                    0.000036,	0.000363,	0.001446,	0.002291,	0.001446,	0.000363,	0.000036
            };

    int origMatrixSize = 7;
    int matrixSize = 0 - (origMatrixSize  - (origMatrixSize - 3));
    int boundarySize = (matrixSize * -1);
    fipImage inputImage;
    inputImage.load("../Images/render_1.png");
    inputImage.convertToFloat();

    unsigned int width = inputImage.getWidth();
    unsigned int height = inputImage.getHeight();
    const float* const inputBuffer = (float*)inputImage.accessPixels();

    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width, height, 24);
    float *outputBuffer = (float*)outputImage.accessPixels();

    auto time = high_resolution_clock::now();


    //Sequential mode
    for(int y = boundarySize; y < height - boundarySize; y++ )
    {
        for(int x = boundarySize; x < width - boundarySize; x++)
        {
            for(int i = matrixSize; i <= (matrixSize * -1); i++)
            {
                for(int k = matrixSize; k <= (matrixSize * -1); k++)
                {
                    if(origMatrixSize == 5)
                    {
                        outputBuffer[y * width + x] =
                                inputBuffer[(y + i) * width + (x + k)] * matrix5[i][k];
                    }
                    else if (origMatrixSize == 7)
                    {
                        outputBuffer[y * width + x] =
                                inputBuffer[(y + i) * width + (x + k)] * matrix7[i][k];
                    }
                    else
                    {
                        outputBuffer[y * width + x] = inputBuffer[y * width + x];
                    }
                }
            }
        }
    }

    auto time2 = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(time2 - time);
    cout << "Sequential Blur took: " << duration.count() << " ms" << "\n";

    time = high_resolution_clock::now();

    parallel_for(blocked_range2d<int, int>(boundarySize, height -
                         boundarySize, boundarySize, width - boundarySize),
                 [=](const blocked_range2d<int, int>& r)
    {
        int yStart = r.rows().begin();
        int yEnd = r.rows().end();
        int xStart = r.cols().begin();
        int xEnd = r.cols().end();

        for (int y = yStart; y < yEnd; ++y)
        {
            for (int x = xStart; x < xEnd; ++x)
            {

                for(int i = matrixSize; i <= (matrixSize * -1); i++)
                {
                    for(int k = matrixSize; k <= (matrixSize * -1); k++)
                    {
                        if(origMatrixSize == 5)
                        {
                            outputBuffer[y * width + x] =
                                    inputBuffer[(y + i) * width + (x + k)] * matrix5[i][k];
                        }
                        else if (origMatrixSize == 7)
                        {
                            outputBuffer[y * width + x] =
                                    inputBuffer[(y + i) * width + (x + k)] * matrix7[i][k];
                        }
                        else
                        {
                            outputBuffer[y * width + x] = inputBuffer[y * width + x];
                        }
                    }
                }
            }
        }

    });

    time2 = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(time2 - time);
    cout << "Parallelized Blur took: " << duration.count() << " ms" << "\n";

    std::cout << "Saving image...\n";

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("grey_blurred.png");

    std::cout << "...done\n\n";
}

void ColourImageProcessing()
{

    int threshold = 1;
    // Setup Input image array
    fipImage inputImage;
    inputImage.load("../Images/render_1.png");
    fipImage inputImage2;
    inputImage2.load("../Images/render_2.png");

    unsigned int width = inputImage.getWidth();
    unsigned int height = inputImage.getHeight();

    // Setup Output image array
    fipImage outputImage;
    outputImage = fipImage(FIT_BITMAP, width, height, 24);

    //2D Vector to hold the RGB colour data of an image
    vector<vector<RGBQUAD>> rgbValues;
    vector<vector<RGBQUAD>> rgbValues2;
    vector<vector<RGBQUAD>> rgbValuesDiff;
    rgbValues.resize(height, vector<RGBQUAD>(width));
    rgbValues2.resize(height, vector<RGBQUAD>(width));
    rgbValuesDiff.resize(height, vector<RGBQUAD>(width));

    RGBQUAD rgb;  //FreeImage structure to hold RGB values of a single pixel
    RGBQUAD rgb2;  //FreeImage structure to hold RGB values of a single pixel
    RGBQUAD diff;

    //SEQUENTIAL



    //Extract colour data from image and store it as individual RGBQUAD elements for every pixel
    for(int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            inputImage.getPixelColor(x, y, &rgb); //Extract pixel(x,y) colour data and place it in rgb
            inputImage2.getPixelColor(x, y, &rgb2); //Extract pixel(x,y) colour data and place it in rgb

            rgbValues[y][x].rgbRed = rgb.rgbRed;
            rgbValues[y][x].rgbGreen = rgb.rgbGreen;
            rgbValues[y][x].rgbBlue = rgb.rgbBlue;

            rgbValues2[y][x].rgbRed = rgb2.rgbRed;
            rgbValues2[y][x].rgbGreen = rgb2.rgbGreen;
            rgbValues2[y][x].rgbBlue = rgb2.rgbBlue;
        }
    }

    auto time = high_resolution_clock::now();

    for(int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            diff.rgbRed = abs(rgbValues2[y][x].rgbRed - rgbValues[y][x].rgbRed);
            diff.rgbGreen= abs(rgbValues2[y][x].rgbGreen - rgbValues[y][x].rgbGreen);
            diff.rgbBlue = abs(rgbValues2[y][x].rgbBlue - rgbValues[y][x].rgbBlue);


            if(diff.rgbRed > threshold && diff.rgbGreen > threshold && diff.rgbBlue > threshold)
            {
                diff.rgbRed = 255;
                diff.rgbBlue = 255;
                diff.rgbGreen = 255;
            }

            else if(diff.rgbRed < threshold && diff.rgbGreen < threshold && diff.rgbBlue < threshold)
            {
                diff.rgbRed = 0;
                diff.rgbBlue = 0;
                diff.rgbGreen = 0;
            }

            rgbValuesDiff[y][x].rgbRed = diff.rgbRed;
            rgbValuesDiff[y][x].rgbGreen = diff.rgbGreen;
            rgbValuesDiff[y][x].rgbBlue = diff.rgbBlue;

            //Place the pixel colour values into output image
            outputImage.setPixelColor(x, y, (&rgbValuesDiff[y][x]));


        }
    }
    auto time2 = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(time2 - time);
    cout << "Sequential Image Processing took: " << duration.count() << " ms" << "\n";

    parallel_for(blocked_range2d<int, int>(0, height, 0, width), [&](const blocked_range2d<int, int>& r)
    {

        auto yStart = r.rows().begin();
        auto yEnd = r.rows().end();
        auto xStart = r.cols().begin();
        auto xEnd = r.cols().end();

        for(int y = yStart; y < yEnd; y++)
        {
            for (int x = xStart; x < xEnd; x++)
            {
                rgbValuesDiff[y][x].rgbRed = (BYTE)abs(rgbValues2[y][x].rgbRed - rgbValues[y][x].rgbRed);
                rgbValuesDiff[y][x].rgbGreen= (BYTE)abs(rgbValues2[y][x].rgbGreen - rgbValues[y][x].rgbGreen);
                rgbValuesDiff[y][x].rgbBlue = (BYTE)abs(rgbValues2[y][x].rgbBlue - rgbValues[y][x].rgbBlue);


                if(rgbValuesDiff[y][x].rgbRed > threshold && rgbValuesDiff[y][x].rgbGreen > threshold && rgbValuesDiff[y][x].rgbBlue > threshold)
                {
                    rgbValuesDiff[y][x].rgbRed = 255;
                    rgbValuesDiff[y][x].rgbBlue = 255;
                    rgbValuesDiff[y][x].rgbGreen = 255;
                }

                else if(rgbValuesDiff[y][x].rgbRed < threshold && rgbValuesDiff[y][x].rgbGreen < threshold && rgbValuesDiff[y][x].rgbBlue < threshold)
                {
                    rgbValuesDiff[y][x].rgbRed = 0;
                    rgbValuesDiff[y][x].rgbBlue = 0;
                    rgbValuesDiff[y][x].rgbGreen = 0;
                }
                outputImage.setPixelColor(x, y, (&rgbValuesDiff[y][x]));
            }
        }
    });

    auto time3 = high_resolution_clock::now();
    auto duration2 = duration_cast<milliseconds>(time3 - time2);
    cout << "Parallel Image Processing took: " << duration2.count() << " ms" << "\n";

    //Save the processed image
    outputImage.save("RGB_processed.png");
}

int main()
{
    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);

    //Part 1 (Greyscale Gaussian blur): -----------DO NOT REMOVE THIS COMMENT----------------------------//

    GaussianBlur();


    //Part 2 (Colour image processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//

    ColourImageProcessing();


    return 0;
}