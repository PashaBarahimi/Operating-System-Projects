#ifndef _SUB_IMAGE_HPP_
#define _SUB_IMAGE_HPP_

#include <cstdint>

namespace img
{
    struct Pixel
    {
        enum Channel
        {
            RED,
            GREEN,
            BLUE
        };
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    };

    class SubImage
    {
    public:
        SubImage(int width, int height, int row, int col);
        SubImage();
        SubImage(const SubImage& other);
        SubImage& operator=(const SubImage& other);
        ~SubImage();

        int width() const;
        int height() const;
        int row() const;
        int col() const;

        Pixel& operator()(int r, int c);
        const Pixel& operator()(int r, int c) const;

    private:
        int width_;
        int height_;
        int row_;
        int col_;
        Pixel* pixels_;

        void copy(const SubImage& other);
    };
} // namespace img

#endif
