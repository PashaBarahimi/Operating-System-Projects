#include "filter.hpp"

#include <algorithm>

namespace img
{
    namespace filter
    {
        void flipHorizontal(SubImage& img)
        {
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width() / 2; ++c)
                    std::swap(img(r, c), img(r, img.width() - c - 1));
        }

        void flipVertical(SubImage& img)
        {
            for (int r = 0; r < img.height() / 2; ++r)
                for (int c = 0; c < img.width(); ++c)
                    std::swap(img(r, c), img(img.height() - r - 1, c));
        }

        void flipDiagonal(SubImage& img)
        {
            SubImage result(img.width(), img.height(), img.row(), img.col());
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width(); ++c)
                    result(c, r) = img(r, c);
            img = result;
        }

        void rotate(SubImage& img, Rotation rotation)
        {
            SubImage result(img.width(), img.height(), img.row(), img.col());
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width(); ++c)
                    if (rotation == Rotation::CLOCKWISE)
                        result(c, img.height() - r - 1) = img(r, c);
                    else
                        result(img.width() - c - 1, r) = img(r, c);
            img = result;
        }

        void grayscale(SubImage& img)
        {
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width(); ++c)
                {
                    uint8_t gray = (img(r, c).red + img(r, c).green + img(r, c).blue) / 3;
                    img(r, c).red = gray;
                    img(r, c).green = gray;
                    img(r, c).blue = gray;
                }
        }

        void invert(SubImage& img)
        {
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width(); ++c)
                {
                    img(r, c).red = 255 - img(r, c).red;
                    img(r, c).green = 255 - img(r, c).green;
                    img(r, c).blue = 255 - img(r, c).blue;
                }
        }

        void binarize(SubImage& img, int threshold)
        {
            for (int r = 0; r < img.height(); ++r)
                for (int c = 0; c < img.width(); ++c)
                {
                    uint8_t gray = (img(r, c).red + img(r, c).green + img(r, c).blue) / 3;
                    if (gray < threshold)
                    {
                        img(r, c).red = 0;
                        img(r, c).green = 0;
                        img(r, c).blue = 0;
                    } else
                    {
                        img(r, c).red = 255;
                        img(r, c).green = 255;
                        img(r, c).blue = 255;
                    }
                }
        }

        void sepia(SubImage& img)
        {
            for (int i = 0; i < img.height(); ++i)
                for (int j = 0; j < img.width(); ++j)
                {
                    int red = img(i, j).red;
                    int green = img(i, j).green;
                    int blue = img(i, j).blue;
                    img(i, j).red = std::min(255, (int) (red * 0.393 + green * 0.769 + blue * 0.189));
                    img(i, j).green = std::min(255, (int) (red * 0.349 + green * 0.686 + blue * 0.168));
                    img(i, j).blue = std::min(255, (int) (red * 0.272 + green * 0.534 + blue * 0.131));
                }
        }

        void diamond(SubImage& img, Pixel color)
        {
            int midWidth = img.width() / 2;
            int midHeight = img.height() / 2;
            drawLine(img, 0, midHeight, midWidth, 0, color);
            drawLine(img, midWidth, 0, img.width() - 1, midHeight, color);
            drawLine(img, img.width() - 1, midHeight, midWidth, img.height() - 1, color);
            drawLine(img, midWidth, img.height() - 1, 0, midHeight, color);
        }

        void blur(SubImage& img, bool fixBorders)
        {
            std::vector<std::vector<double>> kernel = {
                    {1.0 / 16, 2.0 / 16, 1.0 / 16},
                    {2.0 / 16, 4.0 / 16, 2.0 / 16},
                    {1.0 / 16, 2.0 / 16, 1.0 / 16}};
            convolution(img, kernel, fixBorders);
        }

        void sharpen(SubImage& img, bool fixBorders)
        {
            std::vector<std::vector<double>> kernel = {
                    {0,  -1, 0},
                    {-1, 5,  -1},
                    {0,  -1, 0}};
            convolution(img, kernel, fixBorders);
        }

        void emboss(SubImage& img, bool fixBorders)
        {
            std::vector<std::vector<double>> kernel = {
                    {-2, -1, 0},
                    {-1, 1,  1},
                    {0,  1,  2}};
            convolution(img, kernel, fixBorders);
        }

        void detectEdges(SubImage& img, bool fixBorders)
        {
            std::vector<std::vector<double>> kernel = {
                    {-1, -1, -1},
                    {-1, 8,  -1},
                    {-1, -1, -1}};
            convolution(img, kernel, fixBorders);
        }

        void convolution(SubImage& img, const std::vector<std::vector<double>>& kernel, bool fixBorders)
        {
            SubImage result(img.width(), img.height(), img.row(), img.col());
            int kernelSize = static_cast<int>(kernel.size());
            int kernelRadius = kernelSize / 2;

            for (int r = fixBorders ? kernelRadius : 0; r < img.height() - (fixBorders ? kernelRadius : 0); ++r)
                for (int c = fixBorders ? kernelRadius : 0; c < img.width() - (fixBorders ? kernelRadius : 0); ++c)
                {
                    double red = 0;
                    double green = 0;
                    double blue = 0;
                    for (int i = 0; i < kernelSize; ++i)
                        for (int j = 0; j < kernelSize; ++j)
                        {
                            int row = r + i - kernelRadius;
                            int col = c + j - kernelRadius;
                            if (row < 0 || row >= img.height() || col < 0 || col >= img.width())
                                continue;
                            red += img(row, col).red * kernel[i][j];
                            green += img(row, col).green * kernel[i][j];
                            blue += img(row, col).blue * kernel[i][j];
                        }
                    result(r, c).red = std::min(255, std::max(0, (int) red));
                    result(r, c).green = std::min(255, std::max(0, (int) green));
                    result(r, c).blue = std::min(255, std::max(0, (int) blue));
                }
            img = result;
        }

        void drawLine(SubImage& img, int x1, int y1, int x2, int y2, const Pixel& color)
        {
            int dx = x2 - x1;
            int dy = y2 - y1;
            int steps = std::max(abs(dx), abs(dy));
            float xIncrement = static_cast<float>(dx) / (float) steps;
            float yIncrement = static_cast<float>(dy) / (float) steps;
            auto x = static_cast<float>(x1);
            auto y = static_cast<float>(y1);
            for (int i = 0; i <= steps; ++i)
            {
                img((int) y, (int) x) = color;
                x += xIncrement;
                y += yIncrement;
            }
        }
    } // namespace filter
} // namespace img
