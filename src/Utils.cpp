#include "UBG.h" // UBG.h includes Utils.h

#include <random>

int GetRandomInt(int Min, int Max)
{
    ASSERT(Min <= Max);
    static std::random_device RandomDevice{};
    static std::mt19937 MersenneTwisterEngine{ RandomDevice() };

    std::uniform_int_distribution<int> Distrib(Min, Max);

    return Distrib(MersenneTwisterEngine);
}

