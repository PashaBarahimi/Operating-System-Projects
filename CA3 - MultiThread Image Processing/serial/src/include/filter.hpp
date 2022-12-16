#ifndef _FILTER_HPP_
#define _FILTER_HPP_

#include <vector>

#include "bmp24.hpp"

namespace img
{
    enum class Rotation
    {
        CLOCKWISE,
        COUNTER_CLOCKWISE
    };

    BMP24 flipHorizontal(const BMP24& bmp);
    BMP24 flipVertical(const BMP24& bmp);
    BMP24 flipDiagonal(const BMP24& bmp);
    BMP24 rotate(const BMP24& bmp, Rotation rotation);
    BMP24 invert(const BMP24& bmp);
    BMP24 grayscale(const BMP24& bmp);
    BMP24 binarize(const BMP24& bmp, int threshold = 128);
    BMP24 sepia(const BMP24& bmp);
    BMP24 blur(const BMP24& bmp);
    BMP24 diamond(const BMP24& bmp);
    BMP24 sharpen(const BMP24& bmp);
    BMP24 emboss(const BMP24& bmp);
    BMP24 detectEdges(const BMP24& bmp);
    BMP24 convolution(const BMP24& bmp, const std::vector<std::vector<double>>& kernel);
} // namespace img

#endif
