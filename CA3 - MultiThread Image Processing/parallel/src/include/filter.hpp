#ifndef _FILTER_HPP_
#define _FILTER_HPP_

#include <vector>

#include "sub_image.hpp"

namespace img::filter
{
    enum class Rotation
    {
        CLOCKWISE,
        COUNTER_CLOCKWISE
    };

    void flipHorizontal(SubImage& img);
    void flipVertical(SubImage& img);
    void flipDiagonal(SubImage& img);
    void rotate(SubImage& img, Rotation rotation);
    void invert(SubImage& img);
    void grayscale(SubImage& img);
    void binarize(SubImage& img, int threshold = 128);
    void sepia(SubImage& img);
    void diamond(SubImage& img, Pixel color = { 255, 255, 255 });
    void blur(SubImage& img, bool fixBorders = false);
    void sharpen(SubImage& img, bool fixBorders = false);
    void emboss(SubImage& img, bool fixBorders = false);
    void detectEdges(SubImage& img, bool fixBorders = false);
    void convolution(SubImage& img, const std::vector<std::vector<double>>& kernel, bool fixBorders = false);
    void drawLine(SubImage& img, int x1, int y1, int x2, int y2, const Pixel& color);
} // namespace img::filter

#endif