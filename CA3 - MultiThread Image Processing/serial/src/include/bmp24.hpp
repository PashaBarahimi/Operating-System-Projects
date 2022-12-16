#ifndef _BMP24_HPP_
#define _BMP24_HPP_

#include <string>
#include <fstream>
#include <exception>

namespace img
{
constexpr uint16_t BMP_SIGNATURE = 0x4D42; // "BM"

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


#pragma pack(push, 1)
struct BMPHeader
{
    uint16_t signature;  // "BM"
    uint32_t fileSize;   // size of the BMP file in bytes
    uint32_t reserved;   // contains two 16-bit unused values
    uint32_t dataOffset; // offset from the beginning of the file to the bitmap data
};
struct BMPInfoHeader
{
    uint32_t headerSize;      // size of this header in bytes (40)
    int32_t  width;           // width of the bitmap in pixels
    int32_t  height;          // height of the bitmap in pixels
    uint16_t planes;          // number of color planes (must be 1)
    uint16_t bitsPerPixel;    // number of bits per pixel (must be 24)
    uint32_t compression;     // compression method being used
    uint32_t imageSize;       // size of the raw bitmap data in bytes
    int32_t  xPixelsPerMeter; // horizontal resolution of the image in pixels per meter
    int32_t  yPixelsPerMeter; // vertical resolution of the image in pixels per meter
    uint32_t colorsUsed;      // number of colors in the color palette
    uint32_t colorsImportant; // number of important colors used
};
#pragma pack(pop)

class BMP24
{
public:
    BMP24(const std::string& filename);
    BMP24(std::ifstream& file);
    BMP24(const BMP24& other);
    ~BMP24();
    void save(const std::string& filename);
    void save(std::ofstream& file);
    Pixel& operator()(int r, int c);
    Pixel operator()(int r, int c) const;
    int width() const;
    int height() const;

private:
    void load(std::ifstream& file);
    void loadData(std::ifstream& file);
    void write(std::ofstream& file);
    int width_;
    int height_;
    Pixel* data_;
    BMPHeader header_;
    BMPInfoHeader infoHeader_;
};

} // namespace img

#endif
