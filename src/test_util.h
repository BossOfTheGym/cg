#pragma once

#include <utility>

#include "core.h"


template<class Vec, class Gen>
void shuffle(Vec& vec, Gen& gen)
{
    for (i32 i = (i32)vec.size() - 1; i > 0; i--)
    {
        auto j = gen() % (i + 1);

        std::swap(vec[i], vec[j]);
    }
}

template<class Vec, class Gen>
void choose(Vec& vec, Gen& gen)
{
    if (!vec.empty())
    {
        auto i = gen() % vec.size();

        std::swap(vec[i], vec.back());
    }
}
