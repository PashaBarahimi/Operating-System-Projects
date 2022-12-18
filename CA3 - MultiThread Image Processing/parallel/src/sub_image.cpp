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
            if (pixels_ != nullptr)
                delete[] pixels_;
            copy(other);
        }
        return *this;
    }

    SubImage::~SubImage()
    {
        if (pixels_ != nullptr)
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
