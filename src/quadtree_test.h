#pragma once

#include "core.h"

#include <vector>

template<class Vec>
auto create_vecs(const Vec& v0, const Vec& v1, i32 div)
{
	using value = typename Vec::value_type;

	std::vector<Vec> vecs;

	Vec dv = (v1 - v0) / value(div);

	vecs.reserve(div * div);
	for (i32 i = 0; i < div; i++)
	{
		for (i32 j = 0; j < div; j++)
		{
			vecs.push_back(Vec{v0.x + (j + 0.5f) * dv.x, v0.y + (i + 0.5f) * dv.y});
		}
	}

	return vecs;
}


void test_simple_quadtree();
