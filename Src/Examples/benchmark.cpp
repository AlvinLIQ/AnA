#include "Headers/benchmark.hpp"

int main()
{
    benchmark bm{};
    bm.Init();
    bm.Run();
    return 0;
}