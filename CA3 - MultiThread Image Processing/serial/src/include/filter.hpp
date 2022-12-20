#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>

#include "bmp24.hpp"

namespace img
{
    namespace filter
    {
        enum class Rotation
        {
            CLOCKWISE,
            COUNTER_CLOCKWISE
        };

        void flipHorizontal(BMP24& bmp);

        void flipVertical(BMP24& bmp);

        void flipDiagonal(BMP24& bmp);

        void rotate(BMP24& bmp, Rotation rotation);

        void invert(BMP24& bmp);

        void grayscale(BMP24& bmp);

        void binarize(BMP24& bmp, int threshold = 128);

        void sepia(BMP24& bmp);

        void blur(BMP24& bmp);

        void drawLine(BMP24& bmp, int x1, int y1, int x2, int y2, const Pixel& color);

        void diamond(BMP24& bmp);

        void sharpen(BMP24& bmp);

        void emboss(BMP24& bmp);

        void detectEdges(BMP24& bmp);

        void convolution(BMP24& bmp, const std::vector<std::vector<double>>& kernel);
    } // namespace filter
} // namespace img

#endif
