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
        header_ = other.header_;
        infoHeader_ = other.infoHeader_;
        data_ = new Pixel[width_ * height_];
        std::copy(other.data_, other.data_ + width_ * height_, data_);
    }

    BMP24::BMP24(int width, int height)
    {
        width_ = width;
        height_ = height;
        data_ = new Pixel[width_ * height_];
        std::fill(data_, data_ + width_ * height_, Pixel { });
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
            data_ = new Pixel[width_ * height_];
            std::copy(other.data_, other.data_ + width_ * height_, data_);
        }
        return *this;
    }

    void BMP24::load(std::ifstream& file)
    {
        file.read(reinterpret_cast<char*>(&header_), sizeof(BMPHeader));
        if (header_.signature != BMP_SIGNATURE)
            throw std::runtime_error("Invalid BMP file");
        file.read(reinterpret_cast<char*>(&infoHeader_), sizeof(BMPInfoHeader));
        if (infoHeader_.bitsPerPixel != 24)
            throw std::runtime_error("Only 24-bit BMP files are supported");
        width_ = infoHeader_.width;
        height_ = infoHeader_.height;
        loadData(file);
    }

    void BMP24::loadData(std::ifstream& file)
    {
        data_ = new Pixel[width_ * height_];
        file.seekg(header_.dataOffset);
        int padding = (4 - (width_ * sizeof(Pixel)) % 4) % 4;
        for (int y = height_ - 1; y >= 0; --y)
        {
            file.read(reinterpret_cast<char*>(data_ + y * width_), width_ * sizeof(Pixel));
            file.seekg(padding, std::ios::cur);
        }
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
        file.write(reinterpret_cast<char*>(&header_), sizeof(BMPHeader));
        file.write(reinterpret_cast<char*>(&infoHeader_), sizeof(BMPInfoHeader));
        char paddingBuffer[4] = { 0 };
        int padding = (4 - (width_ * sizeof(Pixel)) % 4) % 4;
        for (int y = height_ - 1; y >= 0; --y)
        {
            file.write(reinterpret_cast<char*>(data_ + y * width_), width_ * sizeof(Pixel));
            file.write(paddingBuffer, padding);
        }
    }

    void BMP24::setHeader()
    {
        header_.signature = BMP_SIGNATURE;
        header_.reserved = 0;
        header_.dataOffset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
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

    Pixel& BMP24::operator()(int r, int c)
    {
        return data_[r * width_ + c];
    }

    Pixel BMP24::operator()(int r, int c) const
    {
        return data_[r * width_ + c];
    }

    int BMP24::width() const
    {
        return width_;
    }

    int BMP24::height() const
    {
        return height_;
    }
} // namespace img
