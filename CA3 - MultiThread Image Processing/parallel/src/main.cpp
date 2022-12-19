#include <iostream>
#include <chrono>
#include <vector>
#include <unistd.h>

#include "thread_pool.hpp"
#include "sub_image.hpp"
#include "bmp24.hpp"
#include "filter.hpp"

int TEST_ITERATIONS = 1;
const std::string OUTPUT_FILE = "output.bmp";

void filter(img::BMP24& bmp, int numThreads, ThreadPool<img::SubImage>& pool)
{
    std::vector<img::SubImage> subImages(numThreads);
    std::vector<img::SubImage> borderSubImages(numThreads - 1);
    int rowStart = 0;
    int subImgHeight = bmp.height() / numThreads;
    for (int i = 0; i < numThreads; ++i)
    {
        pool.addTask(i, [=, &bmp](img::SubImage& subImage)
            {
                bmp.createSubImage(rowStart, 0, subImgHeight, bmp.width(), subImage);
            }, subImages[i]);
        if (i != numThreads - 1)
        {
            pool.addTask(i, [=, &bmp](img::SubImage& subImage)
                {
                    bmp.createSubImage(rowStart + subImgHeight - 2, 0, 4, bmp.width(), subImage);
                }, borderSubImages[i]);
        }
        rowStart += subImgHeight;
        if (i == numThreads - 2)
            subImgHeight = bmp.height() - rowStart;
    }

    for (int i = 0; i < numThreads; ++i)
        pool.addTask(i, img::filter::flipHorizontal, subImages[i]);

    for (int i = 0; i < numThreads; ++i)
        pool.addTask(i, [=](img::SubImage& subImage)
            {
                img::filter::emboss(subImage);
            }, subImages[i]);

    for (int i = 0; i < numThreads - 1; ++i)
        pool.addTask(i, [=](img::SubImage& subImage)
            {
                img::filter::flipHorizontal(subImage);
                img::filter::emboss(subImage, true);
            }, borderSubImages[i]);

    for (int i = 0; i < numThreads; ++i)
        pool.addTask(i, [&bmp](img::SubImage& subImage)
            {
                bmp.applySubImage(subImage);
            }, subImages[i]);

    pool.waitAll();

    for (int i = 0; i < numThreads - 1; ++i)
        pool.addTask(i, [&bmp](img::SubImage& subImage)
            {
                subImage.deleteRow(0);
                subImage.deleteRow(subImage.height() - 1);
                bmp.applySubImage(subImage);
            }, borderSubImages[i]);
    pool.waitAll();

     int midWidth = bmp.width() / 2;
     int midHeight = bmp.height() / 2;
     std::vector<img::SubImage> subImages2(4);
     pool.addTask(0, [=, &bmp](img::SubImage& subImage)
         {
             bmp.createSubImage(0, 0, midHeight, midWidth, subImage);
             img::filter::drawLine(subImage, 0, subImage.height() - 1, subImage.width() - 1, 0);
             bmp.applySubImage(subImage);
         }, subImages2[0]);
     pool.addTask([=, &bmp](img::SubImage& subImage)
         {
             bmp.createSubImage(0, midWidth, midHeight, bmp.width() - midWidth, subImage);
             img::filter::drawLine(subImage, 0, 0, subImage.width() - 1, subImage.height() - 1);
             bmp.applySubImage(subImage);
         }, subImages2[1]);
     pool.addTask([=, &bmp](img::SubImage& subImage)
         {
             bmp.createSubImage(midHeight, 0, bmp.height() - midHeight, midWidth, subImage);
             img::filter::drawLine(subImage, 0, 0, subImage.width() - 1, subImage.height() - 1);
             bmp.applySubImage(subImage);
         }, subImages2[2]);
     pool.addTask([=, &bmp](img::SubImage& subImage)
         {
             bmp.createSubImage(midHeight, midWidth, bmp.height() - midHeight, bmp.width() - midWidth, subImage);
             img::filter::drawLine(subImage, 0, subImage.height() - 1, subImage.width() - 1, 0);
             bmp.applySubImage(subImage);
         }, subImages2[3]);
    pool.waitAll();
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

    int numThreads = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    if (numThreads < 4)
        numThreads = 4;

    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        ThreadPool<img::SubImage> pool(numThreads);
        std::cout << "Number of threads: " << numThreads << std::endl;
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            img::BMP24 image(argv[1]);
            filter(image, numThreads, pool);
            image.save(OUTPUT_FILE);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Execution Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / TEST_ITERATIONS << "ms" << std::endl;
        pool.stop();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
