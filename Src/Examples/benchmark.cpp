#include "Headers/benchmark.hpp"

int main(int , char**)
{
    benchmark bm{};
    bm.Init();
    bm.Run();
    return 0;
}
