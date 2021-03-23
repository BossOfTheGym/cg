#pragma once

#include "core.h"
#include "primitive.h"

#include <cassert>
#include <vector>
#include <algorithm>

namespace hull
{
	std::vector<prim::vec2> convex_hull_graham(std::vector<prim::vec2>& vecs, prim::Float eps = 1e-6)
	{
		assert(vecs.size() >= 3);

		auto minElem = std::min_element(vecs.begin(), vecs.end(), 
			[] (const auto& v0, const auto* v1)
			{
				return v0.x < v1.x;
			}
		);
		std::iter_swap(vecs.begin(), minElem);

		std::sort(vecs.begin() + 1, vecs.end(),
			[&] (const auto& v0, const auto& v1)
			{
				auto dv0m = v0 - vecs[0];
				auto dv1m = v1 - vecs[0];

				if (std::abs(dv0m.x) < eps || std::abs(dv1m.x) < eps)
				{
					return dv0m.y < dv1m.y;
				}
				
				auto ang0 = dv0m.y / dv0m.x;
				auto ang1 = dv1m.y / dv1m.x;
				if (std::abs(ang0 - ang1) < eps)
				{
					return dv0m.x < dv1m.x;
				}

				return ang0 < ang1;
			}
		);

		std::vector<prim::vec2> hull;
		for (auto it = vecs.begin(), e = vecs.end(); it != e; it++)
		{	
			while (hull.size() > 1)
			{
				auto top = hull.size() - 1;
				if (!prim::leftTurn(hull[top - 1], hull[top], *it))
				{
					hull.pop_back();
				}
				else
				{
					break;
				}
			}
			hull.push_back(*it);
		}

		return hull;
	}
}