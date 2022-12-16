#include <iostream>
#include <chrono>

#include "bmp24.hpp"
#include "filter.hpp"

const std::string OUTPUT_FILE = "output.bmp";

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return 1;
    }

    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        img::BMP24 bmp(argv[1]);
        img::BMP24 result = img::flipHorizontal(bmp);
        result = img::rasterize(result);
        result = img::diamond(result);
        result.save(OUTPUT_FILE);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
