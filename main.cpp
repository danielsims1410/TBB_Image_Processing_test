//Generic Libraries
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
//Thread building blocks libraries
#include <tbb/task_scheduler_init.h>
#include "tbb/blocked_range2d.h"
//Free Image library
#include <FreeImagePlus.h>

#define MAX_RGB_VALUE 255;

// Part 1 Prototypes
bool PixelsIdentical(RGBQUAD rgb1, RGBQUAD rgb2);
fipImage CompareAndChangeImages(const fipImage& image1, const fipImage& image2);
void MergeImages(const fipImage& image1, const fipImage& image2);

// Part 2 Prototypes
float Gaussian2D(float x1, float y1, float x2, float y2, float sigma);
void ApplyGaussianBlur(fipImage image_to_blur, int kernel_size);
void ThresholdOperation(const fipImage& operating_image);
bool PixelIsColour(RGBQUAD rgb_value, char colour);
bool PixelBelowThreshold(RGBQUAD rgb_value, int threshold);
int RGBValuesSum(RGBQUAD rgb_value);

// Part 3 Prototypes
void CountWhitePixels(const fipImage& image);
void InvertPixelsAtWhitePixels(fipImage& image_to_invert, fipImage& filter_mask_image);

using namespace std;
using namespace tbb;

int main()
{
	int nt = task_scheduler_init::default_num_threads();
	task_scheduler_init T(nt);

	//Part 1 (Image Comparison): -----------------DO NOT REMOVE THIS COMMENT----------------------------//
	cout << "===== PART 1 - IMAGE COMPARISON ====================" << endl;
    fipImage render_top1, render_top2, render_bottom1, render_bottom2;
    fipImage result_image_top, result_image_bottom;

        //Load Images
    render_top1.load("../Images/render_top_1.png");
    render_top2.load("../Images/render_top_2.png");
    render_bottom1.load("../Images/render_bottom_1.png");
    render_bottom2.load("../Images/render_bottom_2.png");

        //Compare Pixels and Create new Images
    cout << "--- Compare Images & Change ----------" << endl;
    cout << "Comparing Top Pair Images..." << endl;
    result_image_top = CompareAndChangeImages(render_top1, render_top2);
    cout << "Saving stage1_top.png..." << endl << endl;
    result_image_top.save("../Images/stage1_top.png");
    cout << "Comparing Bottom Pair Images..." << endl;
    result_image_bottom = CompareAndChangeImages(render_bottom1, render_bottom2);
    cout << "Saving stage1_bottom.png..." << endl << endl;
    result_image_bottom.save("../Images/stage1_bottom.png");

        //Combine Resulting Images
        cout << "--- Merging Resulting Images ----------" << endl;
    MergeImages(result_image_top, result_image_bottom);


	//Part 2 (Blur & post-processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//
	cout << "===== PART 2 - BLUR & POST-PROCESSING ====================" << endl;
    fipImage stage1_combined, stage2_blurred;
    stage1_combined.load("../Images/stage1_combined.png");

        //Add Gaussian Blur to Image
    cout << "--- Adding Gaussian Blur to stage1_combined.png..." << endl;
    int kernel_size;
    cout << "Enter a Kernel Size for Stencil Pattern: ";
    cin >> kernel_size;
    ApplyGaussianBlur(stage1_combined, kernel_size);

        //Perform Threshold Operation
    cout << "--- Performing Threshold Operation ----------" << endl;
    stage2_blurred.load("../Images/stage2_blurred.png");
    ThresholdOperation(stage2_blurred);


	//Part 3 (Image Mask): -----------------------DO NOT REMOVE THIS COMMENT----------------------------//
	cout << "===== PART 3 - IMAGE MASK ===================" << endl;
    fipImage stage2_threshold; //render_top1 already loaded (l38)
    stage2_threshold.load("../Images/stage2_threshold.png");

        //Count White Pixels & Calculate Percentage
    cout << "--- Counting White Pixels ----------" << endl;
    CountWhitePixels(stage2_threshold);

        //Filter Mask & Invert
    cout << "--- Inverting Image Using Filter Mask ----------" << endl;
    InvertPixelsAtWhitePixels(render_top1, stage2_threshold);
}

//P1
fipImage CompareAndChangeImages(const fipImage& image1, const fipImage& image2) {
    fipImage result_image;
    unsigned int height = image1.getHeight();
    unsigned int width = image1.getWidth();

    result_image = fipImage(FIT_BITMAP, width,height, 24);

    RGBQUAD rgb; //rgb value to be used for each pixel
    RGBQUAD rgb2;
    vector<vector<RGBQUAD>> new_rgb_values;
    new_rgb_values.resize(height, vector<RGBQUAD>(width));
    int colour_value;

    cout << "Finding Identical Pixels..." << endl;
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image1.getPixelColor(j, i, &rgb);
            image2.getPixelColor(j, i, &rgb2);

            PixelsIdentical(rgb, rgb2) ? colour_value = 0 : colour_value = MAX_RGB_VALUE;
            new_rgb_values[i][j].rgbRed = colour_value;
            new_rgb_values[i][j].rgbGreen = colour_value;
            new_rgb_values[i][j].rgbBlue = colour_value;

            result_image.setPixelColor(j, i, &new_rgb_values[i][j]);
        }
    }
    cout << "Identical Pixels Found..." << endl;
    result_image.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    result_image.convertTo24Bits();
    return result_image;
}

//P1
bool PixelsIdentical(RGBQUAD rgb1, RGBQUAD rgb2) {
    bool identical = false;
    if(rgb1.rgbRed == rgb2.rgbRed) {
        if(rgb1.rgbGreen == rgb2.rgbGreen) {
            if(rgb1.rgbBlue == rgb2.rgbBlue) {
                identical = true;
            }
        }
    }
    return identical;
}

//P1
void MergeImages(const fipImage& image1, const fipImage& image2) {
    unsigned int height = image1.getHeight();
    unsigned int width = image1.getWidth();
    fipImage result_image = fipImage(FIT_BITMAP, width, height, 24);

    RGBQUAD rgb_value;
    vector<vector<RGBQUAD>> new_rgb_values;
    new_rgb_values.resize(height, vector<RGBQUAD>(width));

    cout << "Bottom half of stage1_bottom.png being copied..." << endl;
    for(unsigned int i = 0; i < height/2; i++) {
        for(int j = 0; j < width; j++) {
            image2.getPixelColor(j, i, &rgb_value);
            new_rgb_values[i][j].rgbRed = rgb_value.rgbRed;
            new_rgb_values[i][j].rgbGreen = rgb_value.rgbGreen;
            new_rgb_values[i][j].rgbBlue = rgb_value.rgbBlue;
            result_image.setPixelColor(j, i, &new_rgb_values[i][j]);
        }
    }

    cout << "Top Half of stage1_top.png being copied..." << endl;
    for(unsigned int i = height/2; i < height; i++) {
        for(int j = 0; j < width; j++) {
            image1.getPixelColor(j, i, &rgb_value);
            new_rgb_values[i][j].rgbRed = rgb_value.rgbRed;
            new_rgb_values[i][j].rgbGreen = rgb_value.rgbGreen;
            new_rgb_values[i][j].rgbBlue = rgb_value.rgbBlue;
            result_image.setPixelColor(j, i, &new_rgb_values[i][j]);
        }
    }
    cout << "Images Merged!" << endl;
    result_image.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    result_image.convertTo24Bits();
    cout << "Saving stage1_combined.png..." << endl << endl;
    result_image.save("../Images/stage1_combined.png");
}

//P2
void ApplyGaussianBlur(fipImage image_to_blur, int kernel_size) {
    image_to_blur.convertToFloat();
    unsigned int height = image_to_blur.getHeight();
    unsigned int width = image_to_blur.getWidth();
    const float* input_buffer = (float*)image_to_blur.accessPixels();

    fipImage blurred_image = fipImage(FIT_FLOAT, width, height, 32);
    auto *blurred_image_buffer = (float*)blurred_image.accessPixels();


    auto blur = [&](const blocked_range2d<int, int>& r) {
        auto y_begin = r.rows().begin();
        auto y_end = r.rows().end();
        auto x_begin = r.cols().begin();
        auto x_end = r.cols().end();

        int starting_x;
        int starting_y;
        float sigma;
        cout << "Enter a value for Sigma: ";
        cin >> sigma;
        cout << "Applying Gaussian Blur with Stencil Pattern at these coordinates..." << endl;
        for(int y = y_begin; y < y_end; ++y) {
            for (int x = x_begin; x < x_end; ++x) {
                float new_pixel_value = 0.0f;

                if (x <= kernel_size) starting_x = 0;
                else starting_x = kernel_size;

                if (y <= kernel_size) starting_y = 0;
                else starting_y = kernel_size;

                for (int x2 = x - starting_x; x2 <= (x + kernel_size); x2++) {
                    for (int y2 = y - starting_y; y2 <= (y + kernel_size); y2++) {
                        //cout << endl << "newY: " << y2 << " | newX: " << x2
                        //<< "| x: " << x << " | y: " << y;
                        if ((y2 > 0 && y2 < height) && (x2 > 0 && x2 < width)) {
                            new_pixel_value += (Gaussian2D(x, y, x2, y2, sigma) * input_buffer[y2 * width + x2]);
                        } else {
                            new_pixel_value += (Gaussian2D(x, y, x, y, sigma) * input_buffer[y * width + x]);
                        }
                    }
                }
                blurred_image_buffer[y * width + x] = new_pixel_value;
            }
        }
    };

    blur(blocked_range2d<int, int>(0, height, 0, width));
    cout << "Gaussian Blur Applied!" << endl;

    blurred_image.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    blurred_image.convertTo32Bits();
    blurred_image.save("../Images/stage2_blurred.png");
    cout << "Saving stage2_blurred.png..." << endl << endl;
}

//P2
float Gaussian2D(float x1, float y1, float x2, float y2, float sigma) {
    float sigma_squared = sigma * sigma;
    float xSquared = (x1 - x2) * (x1 - x2);
    float ySquared = (y1 - y2) * (y1 - y2);
    return (1.0f / (2.0f * M_PI * sigma_squared)) * (exp(-((xSquared + ySquared) / (2.0f * sigma_squared))));
}

//P2
void ThresholdOperation(const fipImage& operating_image) {
    unsigned int height = operating_image.getHeight();
    unsigned int width = operating_image.getWidth();

    fipImage result_image = fipImage(FIT_BITMAP, width, height, 24);

    RGBQUAD rgb_value;
    vector<vector<RGBQUAD>> new_rgb_values;
    new_rgb_values.resize(height, vector<RGBQUAD>(width));
    int threshold;
    cout << "Enter a Threshold: ";
    cin >> threshold;

    cout << "Applying Threshold..." << endl;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            operating_image.getPixelColor(j, i, &rgb_value);
            if (PixelIsColour(rgb_value, 'b') || (!PixelBelowThreshold(rgb_value, threshold))) {
                new_rgb_values[i][j].rgbRed = rgb_value.rgbRed;
                new_rgb_values[i][j].rgbGreen = rgb_value.rgbGreen;
                new_rgb_values[i][j].rgbBlue = rgb_value.rgbBlue;
                //-> Don't change anything
            }

            else {
                new_rgb_values[i][j].rgbRed = MAX_RGB_VALUE;
                new_rgb_values[i][j].rgbGreen = MAX_RGB_VALUE;
                new_rgb_values[i][j].rgbBlue = MAX_RGB_VALUE;
            }

            result_image.setPixelColor(j, i, &new_rgb_values[i][j]);
        }
    }
    cout << "Threshold Applied!" << endl;
    result_image.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    result_image.convertTo24Bits();
    cout << "Saving stage2_threshold.png..." << endl << endl;
    result_image.save("../Images/stage2_threshold.png");
}
//P2
bool PixelBelowThreshold(RGBQUAD rgb_value, int threshold) {
    bool is_below;
    RGBValuesSum(rgb_value) < threshold ? is_below = true : is_below = false;
    return is_below;
}

//P2
int RGBValuesSum(RGBQUAD rgb_value) {
    return rgb_value.rgbBlue + rgb_value.rgbGreen + rgb_value.rgbBlue;
}

//P3
void CountWhitePixels(const fipImage& image) {
    unsigned int height = image.getHeight();
    unsigned int width = image.getWidth();
    float total_pixels = height * width;

    RGBQUAD rgb_value;
    cout << "Counting White Pixels..." << endl;
    int white_pixel_count = 0;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            image.getPixelColor(j, i, &rgb_value);
            if(PixelIsColour(rgb_value, 'w')) ++white_pixel_count;
        }
    }
    float white_pixel_percentage = (white_pixel_count / total_pixels) * 100.0f;
    cout << "Number of White Pixels: " << white_pixel_count << endl;
    cout << "Percentage: " << white_pixel_percentage << "% of Pixels" << endl << endl;
}

//P2 & P3
bool PixelIsColour(RGBQUAD rgb_value, char colour) {
    bool is_colour;
    int required_rgb_value = -1;
    switch(colour) {
        case('b'):
            required_rgb_value = 0;
            break;
        case('w'):
            required_rgb_value = 255 * 3;
            break;
        default:
            break;
    }

    (rgb_value.rgbRed + rgb_value.rgbGreen + rgb_value.rgbBlue
        == required_rgb_value)? is_colour = true : is_colour = false;
    return is_colour;
}

//P3
void InvertPixelsAtWhitePixels(fipImage& image_to_invert, fipImage& filter_mask_image) {
    unsigned int height = image_to_invert.getHeight();
    unsigned int width = image_to_invert.getWidth();
    vector<int> white_x_coordinates;
    vector<int> white_y_coordinates;
    RGBQUAD rgb_value;
    cout << "Searching for & Storing White Pixel Coordinates in stage2_threshold.png..." << endl;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            filter_mask_image.getPixelColor(j, i, &rgb_value);
            if(PixelIsColour(rgb_value, 'w')) {
                //Log coordinates of white pixels
                white_x_coordinates.push_back(j);
                white_y_coordinates.push_back(i);
            }
        }
    }

    cout << "Inverting Pixels in render_top1.png at these coordinates..." << endl;
    RGBQUAD rgb_value_invert;
    int vector_size = white_x_coordinates.size();
    for(int i = 0; i < vector_size; i++) {
        image_to_invert.getPixelColor(white_x_coordinates.at(i), white_y_coordinates.at(i), &rgb_value_invert);
        rgb_value_invert.rgbRed = 255 - rgb_value_invert.rgbRed;
        rgb_value_invert.rgbGreen = 255 - rgb_value_invert.rgbGreen;
        rgb_value_invert.rgbBlue = 255 - rgb_value_invert.rgbBlue;
        image_to_invert.setPixelColor(white_x_coordinates.at(i), white_y_coordinates.at(i), &rgb_value_invert);
    }
    cout << "Saving stage3_invert.png..." << endl;
    image_to_invert.save("../Images/stage3_invert.png");
    //
}

