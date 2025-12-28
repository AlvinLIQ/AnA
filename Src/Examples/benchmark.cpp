#include "Headers/benchmark.hpp"

int main(int argc, char** argv)
{
    benchmark bm{};
    bm.Init();
    bm.Run();
    return 0;
}