#include "sub_image.hpp"

#include <algorithm>

namespace img
{
    SubImage::SubImage(int width, int height, int row, int col)
        : width_(width), height_(height), row_(row), col_(col)
    {
        pixels_ = new Pixel[width_ * height_];
    }

    SubImage::SubImage()
        : width_(0), height_(0), row_(0), col_(0)
    {
        pixels_ = nullptr;
    }

    SubImage::SubImage(const SubImage& other)
    {
        if (this != &other)
            copy(other);
    }

    SubImage& SubImage::operator=(const SubImage& other)
    {
        if (this != &other)
        {
            delete[] pixels_;
            copy(other);
        }
        return *this;
    }

    SubImage::~SubImage()
    {
        delete[] pixels_;
    }

    int SubImage::width() const
    {
        return width_;
    }

    int SubImage::height() const
    {
        return height_;
    }

    int SubImage::row() const
    {
        return row_;
    }

    int SubImage::col() const
    {
        return col_;
    }

    Pixel& SubImage::operator()(int r, int c)
    {
        return pixels_[r * width_ + c];
    }

    const Pixel& SubImage::operator()(int r, int c) const
    {
        return pixels_[r * width_ + c];
    }

    void SubImage::deleteRow(int r)
    {
        for (int i = r; i < height_ - 1; ++i)
            std::copy(pixels_ + (i + 1) * width_, pixels_ + (i + 2) * width_, pixels_ + i * width_);
        if (r == 0)
            row_ += 1;
        --height_;
    }

    void SubImage::deleteCol(int c)
    {
        for (int i = c; i < width_ - 1; ++i)
            for (int j = 0; j < height_; ++j)
                pixels_[j * width_ + i] = pixels_[j * width_ + i + 1];
        if (c == 0)
            col_ += 1;
        --width_;
    }

    void SubImage::copy(const SubImage& other)
    {
        width_ = other.width_;
        height_ = other.height_;
        row_ = other.row_;
        col_ = other.col_;
        pixels_ = new Pixel[width_ * height_];
        std::copy(other.pixels_, other.pixels_ + width_ * height_, pixels_);
    }
} // namespace img
