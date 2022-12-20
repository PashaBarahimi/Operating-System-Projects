#include <iostream>
#include <chrono>

#include "bmp24.hpp"
#include "filter.hpp"

int TEST_ITERATIONS = 1;
const std::string OUTPUT_FILE = "output.bmp";

void executionTime(
        std::chrono::time_point<std::chrono::high_resolution_clock>& start,
        const std::string& name, bool fromStart = false, int iterations = 1)
{
    static auto s = start;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - (fromStart ? s : start);
    std::cout << name << ": " << elapsed.count() / iterations << " ms" << std::endl;
    start = end;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
        return 1;
    }

    if (argc > 2)
        TEST_ITERATIONS = std::stoi(argv[2]);

    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            img::BMP24 bmp(argv[1]);
            // executionTime(start, "File Load");
            img::filter::flipHorizontal(bmp);
            // executionTime(start, "Flip Horizontal");
            img::filter::emboss(bmp);
            // executionTime(start, "Emboss");
            img::filter::diamond(bmp);
            // executionTime(start, "Diamond");
            bmp.save(OUTPUT_FILE);
            // executionTime(start, "File Save");
        }
        executionTime(start, "Execution Time", true, TEST_ITERATIONS);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
