#include "filter.hpp"

#include <algorithm>

namespace img::filter
{
    BMP24 flipHorizontal(const BMP24& bmp)
    {
        BMP24 result(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width() / 2; ++j)
                std::swap(result(i, j), result(i, bmp.width() - j - 1));
        return result;
    }

    BMP24 flipVertical(const BMP24& bmp)
    {
        BMP24 result(bmp);
        for (int i = 0; i < bmp.height() / 2; ++i)
            for (int j = 0; j < bmp.width(); ++j)
                std::swap(result(i, j), result(bmp.height() - i - 1, j));
        return result;
    }

    BMP24 flipDiagonal(const BMP24& bmp)
    {
        BMP24 result(bmp.height(), bmp.width());
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
                result(j, i) = bmp(i, j);
        return result;
    }

    BMP24 rotate(const BMP24& bmp, Rotation rotation)
    {
        BMP24 result(bmp.height(), bmp.width());
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
                if (rotation == Rotation::CLOCKWISE)
                    result(j, bmp.height() - i - 1) = bmp(i, j);
                else
                    result(bmp.width() - j - 1, i) = bmp(i, j);
        return result;
    }

    BMP24 invert(const BMP24& bmp)
    {
        BMP24 result(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                result(i, j).red = 255 - result(i, j).red;
                result(i, j).green = 255 - result(i, j).green;
                result(i, j).blue = 255 - result(i, j).blue;
            }
        return result;
    }

    BMP24 grayscale(const BMP24& bmp)
    {
        BMP24 result(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                int gray = (result(i, j).red + result(i, j).green + result(i, j).blue) / 3;
                result(i, j).red = gray;
                result(i, j).green = gray;
                result(i, j).blue = gray;
            }
        return result;
    }

    BMP24 binarize(const BMP24& bmp, int threshold)
    {
        BMP24 result = grayscale(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                if (result(i, j).red < threshold)
                {
                    result(i, j).red = 0;
                    result(i, j).green = 0;
                    result(i, j).blue = 0;
                }
                else
                {
                    result(i, j).red = 255;
                    result(i, j).green = 255;
                    result(i, j).blue = 255;
                }
            }
        return result;
    }

    BMP24 sepia(const BMP24& bmp)
    {
        BMP24 result(bmp);
        for (int i = 0; i < bmp.height(); ++i)
            for (int j = 0; j < bmp.width(); ++j)
            {
                int red = result(i, j).red;
                int green = result(i, j).green;
                int blue = result(i, j).blue;
                result(i, j).red = std::min(255, (int)(red * 0.393 + green * 0.769 + blue * 0.189));
                result(i, j).green = std::min(255, (int)(red * 0.349 + green * 0.686 + blue * 0.168));
                result(i, j).blue = std::min(255, (int)(red * 0.272 + green * 0.534 + blue * 0.131));
            }
        return result;
    }

    BMP24 blur(const BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { 1.0 / 16, 2.0 / 16, 1.0 / 16 },
            { 2.0 / 16, 4.0 / 16, 2.0 / 16 },
            { 1.0 / 16, 2.0 / 16, 1.0 / 16 } };
        return convolution(bmp, kernel);
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

    BMP24 diamond(const BMP24& bmp)
    {
        BMP24 result(bmp);
        int midWidth = bmp.width() / 2;
        int midHeight = bmp.height() / 2;
        Pixel white = { 255, 255, 255 };
        drawLine(result, 0, midHeight, midWidth, 0, white);
        drawLine(result, midWidth, 0, bmp.width() - 1, midHeight, white);
        drawLine(result, bmp.width() - 1, midHeight, midWidth, bmp.height() - 1, white);
        drawLine(result, midWidth, bmp.height() - 1, 0, midHeight, white);
        return result;
    }

    BMP24 sharpen(const BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { 0, -1, 0 },
            { -1, 5, -1 },
            { 0, -1, 0 } };
        return convolution(bmp, kernel);
    }

    BMP24 emboss(const BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { -2, -1, 0 },
            { -1, 1, 1 },
            { 0, 1, 2 } };
        return convolution(bmp, kernel);
    }

    BMP24 detectEdges(const BMP24& bmp)
    {
        std::vector<std::vector<double>> kernel = {
            { -1, -1, -1 },
            { -1, 8, -1 },
            { -1, -1, -1 } };
        return convolution(bmp, kernel);
    }

    BMP24 convolution(const BMP24& bmp, const std::vector<std::vector<double>>& kernel)
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
        return result;
    }
} // namespace img::filter
