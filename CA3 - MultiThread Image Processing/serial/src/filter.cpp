#include "filter.hpp"

#include <algorithm>

namespace img::filter
{
    void flipHorizontal(BMP24& bmp)
    {
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width() / 2; ++j)
                std::swap(bmp(i, j), bmp(i, bmp.width() - j - 1));
    }

    void flipVertical(BMP24& bmp)
    {
        for (int i = 0; i < bmp.height() / 2; ++i)
            for (int j = 0; j < bmp.width(); ++j)
                std::swap(bmp(i, j), bmp(bmp.height() - i - 1, j));
    }

    void flipDiagonal(BMP24& bmp)
    {
        BMP24 result(bmp.height(), bmp.width());
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
                result(j, i) = bmp(i, j);
        bmp = result;
    }

    void rotate(BMP24& bmp, Rotation rotation)
    {
        BMP24 result(bmp.height(), bmp.width());
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
                if (rotation == Rotation::CLOCKWISE)
                    result(j, bmp.height() - i - 1) = bmp(i, j);
                else
                    result(bmp.width() - j - 1, i) = bmp(i, j);
        bmp = result;
    }

    void invert(BMP24& bmp)
    {
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                bmp(i, j).red = 255 - bmp(i, j).red;
                bmp(i, j).green = 255 - bmp(i, j).green;
                bmp(i, j).blue = 255 - bmp(i, j).blue;
            }
    }

    void grayscale(BMP24& bmp)
    {
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                int gray = (bmp(i, j).red + bmp(i, j).green + bmp(i, j).blue) / 3;
                bmp(i, j).red = gray;
                bmp(i, j).green = gray;
                bmp(i, j).blue = gray;
            }
    }

    void binarize(BMP24& bmp, int threshold)
    {
        grayscale(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                if (bmp(i, j).red < threshold)
                {
                    bmp(i, j).red = 0;
                    bmp(i, j).green = 0;
                    bmp(i, j).blue = 0;
                }
                else
                {
                    bmp(i, j).red = 255;
                    bmp(i, j).green = 255;
                    bmp(i, j).blue = 255;
                }
            }
    }

    void sepia(BMP24& bmp)
    {
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                int red = bmp(i, j).red;
                int green = bmp(i, j).green;
                int blue = bmp(i, j).blue;
                bmp(i, j).red = std::min(255, (int)(red * 0.393 + green * 0.769 + blue * 0.189));
                bmp(i, j).green = std::min(255, (int)(red * 0.349 + green * 0.686 + blue * 0.168));
                bmp(i, j).blue = std::min(255, (int)(red * 0.272 + green * 0.534 + blue * 0.131));
            }
    }

    void blur(BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { 1.0 / 16, 2.0 / 16, 1.0 / 16 },
            { 2.0 / 16, 4.0 / 16, 2.0 / 16 },
            { 1.0 / 16, 2.0 / 16, 1.0 / 16 } };
        convolution(bmp, kernel);
    }

    void drawLine(BMP24& bmp, int x1, int y1, int x2, int y2, const Pixel& color)
    {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int steps = std::max(abs(dx), abs(dy));
        float xIncrement = dx / (float)steps;
        float yIncrement = dy / (float)steps;
        float x = x1;
        float y = y1;
        for (int i = 0; i <= steps; ++i)
        {
            bmp((int)y, (int)x) = color;
            x += xIncrement;
            y += yIncrement;
        }
    }

    void diamond(BMP24& bmp)
    {
        int midWidth = bmp.width() / 2;
        int midHeight = bmp.height() / 2;
        Pixel white = { 255, 255, 255 };
        drawLine(bmp, 0, midHeight, midWidth, 0, white);
        drawLine(bmp, midWidth, 0, bmp.width() - 1, midHeight, white);
        drawLine(bmp, bmp.width() - 1, midHeight, midWidth, bmp.height() - 1, white);
        drawLine(bmp, midWidth, bmp.height() - 1, 0, midHeight, white);
    }

    void sharpen(BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { 0, -1, 0 },
            { -1, 5, -1 },
            { 0, -1, 0 } };
        convolution(bmp, kernel);
    }

    void emboss(BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { -2, -1, 0 },
            { -1, 1, 1 },
            { 0, 1, 2 } };
        convolution(bmp, kernel);
    }

    void detectEdges(BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { -1, -1, -1 },
            { -1, 8, -1 },
            { -1, -1, -1 } };
        convolution(bmp, kernel);
    }

    void convolution(BMP24& bmp, const std::vector<std::vector<double>>& kernel)
    {
        BMP24 result(bmp);
        int kernelSize = kernel.size();
        int kernelRadius = kernelSize / 2;

        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                double red = 0;
                double green = 0;
                double blue = 0;
                for (int k = 0; k < kernelSize; ++k)
                    for (int l = 0; l < kernelSize; ++l)
                    {
                        int x = j + l - kernelRadius;
                        int y = i + k - kernelRadius;
                        if (x < 0 || x >= bmp.width() || y < 0 || y >= bmp.height())
                            continue;
                        red += bmp(y, x).red * kernel[k][l];
                        green += bmp(y, x).green * kernel[k][l];
                        blue += bmp(y, x).blue * kernel[k][l];
                    }
                result(i, j).red = std::min(255, std::max(0, (int)red));
                result(i, j).green = std::min(255, std::max(0, (int)green));
                result(i, j).blue = std::min(255, std::max(0, (int)blue));
            }
        bmp = result;
    }
} // namespace img::filter
