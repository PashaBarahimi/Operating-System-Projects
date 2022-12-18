#include <iostream>

#include <unistd.h>

#include "thread_pool.hpp"
#include "bmp24.hpp"
#include "sub_image.hpp"
#include "filter.hpp"

int main()
{
    // int numCpu = sysconf(_SC_NPROCESSORS_ONLN);
    img::BMP24 image("input.bmp");
    img::SubImage subImg;
    image.createSubImage(0, 0, 356, 356, subImg);
    img::filter::detectEdges(subImg);
    image.applySubImage(subImg);
    image.save("output.bmp");
    return 0;
}
