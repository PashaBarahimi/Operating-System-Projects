#include <iostream>
#include <chrono>

#include "bmp24.hpp"
#include "filter.hpp"

constexpr int TEST_ITERATIONS = 1;
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
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            img::BMP24 bmp(argv[1]);
            img::BMP24 result = img::filter::flipHorizontal(bmp);
            result = img::filter::emboss(result);
            result = img::filter::diamond(result);
            result.save(OUTPUT_FILE);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Executation Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / TEST_ITERATIONS << "ms" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
