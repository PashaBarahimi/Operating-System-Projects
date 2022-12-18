#include "bmp24.hpp"

namespace img
{
    BMP24::BMP24(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Could not open file " + filename);
        load(file);
    }

    BMP24::BMP24(std::ifstream& file)
    {
        if (!file.is_open())
            throw std::runtime_error("File is not open");
        load(file);
    }

    BMP24::BMP24(const BMP24& other)
    {
        width_ = other.width_;
        height_ = other.height_;
        padding_ = other.padding_;
        header_ = other.header_;
        infoHeader_ = other.infoHeader_;
        data_ = new uint8_t[(width_ * height_ * 3) + (padding_ * height_)];
        std::copy(other.data_, other.data_ + (width_ * height_ * 3) + (padding_ * height_), data_);
    }

    BMP24::BMP24(int width, int height)
        : width_(width), height_(height)
    {
        padding_ = (4 - (width_ * 3) % 4) % 4;
        data_ = new uint8_t[(width_ * height_ * 3) + (padding_ * height_)];
        std::fill(data_, data_ + (width_ * height_ * 3) + (padding_ * height_), 0);
        setHeader();
    }

    BMP24::~BMP24()
    {
        delete[] data_;
    }

    BMP24 BMP24::operator=(const BMP24& other)
    {
        if (this != &other)
        {
            delete[] data_;
            width_ = other.width_;
            height_ = other.height_;
            header_ = other.header_;
            infoHeader_ = other.infoHeader_;
            data_ = new uint8_t[(width_ * height_ * 3) + (padding_ * height_)];
            std::copy(other.data_, other.data_ + (width_ * height_ * 3) + (padding_ * height_), data_);
        }
        return *this;
    }

    Pixel& BMP24::operator()(int r, int c)
    {
        int backwardRow = height_ - r - 1;
        return *reinterpret_cast<Pixel*>(data_ + (backwardRow * (width_ * 3 + padding_)) + (c * 3));
    }

    Pixel BMP24::operator()(int r, int c) const
    {
        int backwardRow = height_ - r - 1;
        return *reinterpret_cast<Pixel*>(data_ + (backwardRow * (width_ * 3 + padding_)) + (c * 3));
    }

    void BMP24::save(const std::string& filename)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Could not open file " + filename);
        save(file);
    }

    void BMP24::save(std::ofstream& file)
    {
        if (!file.is_open())
            throw std::runtime_error("File is not open");
        file.write(reinterpret_cast<char*>(&header_), sizeof(header_));
        file.write(reinterpret_cast<char*>(&infoHeader_), sizeof(infoHeader_));
        file.write(reinterpret_cast<char*>(data_), (width_ * height_ * 3) + (padding_ * height_));
    }

    void BMP24::createSubImage(int startRow, int startCol, int height, int width, SubImage& subImage)
    {
        if (startRow + height > height_ || startCol + width > width_)
            throw std::runtime_error("SubImage is out of bounds");
        subImage = SubImage(width, height, startRow, startCol);
        for (int r = 0; r < height; ++r)
            for (int c = 0; c < width; ++c)
                subImage(r, c) = (*this)(startRow + r, startCol + c);
    }

    void BMP24::applySubImage(const SubImage& subImage)
    {
        if (subImage.row() + subImage.height() > height_ || subImage.col() + subImage.width() > width_)
            throw std::runtime_error("SubImage is out of bounds");
        for (int r = 0; r < subImage.height(); ++r)
            for (int c = 0; c < subImage.width(); ++c)
                (*this)(subImage.row() + r, subImage.col() + c) = subImage(r, c);
    }

    int BMP24::width() const
    {
        return width_;
    }

    int BMP24::height() const
    {
        return height_;
    }

    void BMP24::load(std::ifstream& file)
    {
        file.read(reinterpret_cast<char*>(&header_), sizeof(header_));
        file.read(reinterpret_cast<char*>(&infoHeader_), sizeof(infoHeader_));
        width_ = infoHeader_.width;
        height_ = infoHeader_.height;
        padding_ = (4 - (width_ * 3) % 4) % 4;
        data_ = new uint8_t[(width_ * height_ * 3) + (padding_ * height_)];
        file.seekg(header_.dataOffset);
        file.read(reinterpret_cast<char*>(data_), (width_ * height_ * 3) + (padding_ * height_));
    }

    void BMP24::setHeader()
    {
        header_.signature = BMP_SIGNATURE;
        header_.reserved = 0;
        header_.dataOffset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
        header_.fileSize = header_.dataOffset + (width_ * height_ * 3) + (padding_ * height_);
        infoHeader_.headerSize = sizeof(BMPInfoHeader);
        infoHeader_.width = width_;
        infoHeader_.height = height_;
        infoHeader_.planes = 1;
        infoHeader_.bitsPerPixel = 24;
        infoHeader_.compression = 0;
        infoHeader_.imageSize = 0;
        infoHeader_.xPixelsPerMeter = 0;
        infoHeader_.yPixelsPerMeter = 0;
        infoHeader_.colorsUsed = 0;
        infoHeader_.colorsImportant = 0;
    }
} // namespace bmp
