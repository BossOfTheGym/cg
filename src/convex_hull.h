#pragma once

#include "core.h"
#include "primitive.h"

#include <vector>
#include <cassert>
#include <utility>
#include <algorithm>

namespace hull
{
	using namespace prim;

	// TODO : rename, remove hull from name(I have namespace already)
	template<class handle_t, class sampler_t>
	std::vector<handle_t> convex_hull_graham(std::vector<handle_t>& vecs, sampler_t sampler, Float eps = default_eps)
	{
		if (vecs.size() <= 3)
			return vecs;

		auto leftmost   = [&](auto v0, auto v1)
		{
			auto vv0 = sampler(v0);
			auto vv1 = sampler(v1);
			if (std::abs(vv0.x - vv1.x) > eps)
				return vv0.x < vv1.x;
			return std::abs(vv0.y - vv1.y) > eps && vv0.y < vv1.y;
		};
		auto rightmost  = [&](auto v0, auto v1){ return sampler(v0).x > sampler(v1).x; };
		auto topmost    = [&](auto v0, auto v1){ return sampler(v0).y > sampler(v1).y; };
		auto bottommost = [&](auto v0, auto v1){ return sampler(v0).y < sampler(v1).y; };

		auto refine = [&](auto first, auto last, auto v0, auto v1, auto v2)
		{
			auto tri = Triangle2::construct(sampler(v0), sampler(v1), sampler(v2));
			auto it = std::remove_if(first, last, [&](auto v){ return inTriangle(tri, sampler(v)); });
			vecs.erase(it, vecs.end());
		};

		auto pred = [&](auto v0, auto v1)
		{
			auto vv  = sampler(vecs[0]);
			auto vv0 = sampler(v0);
			auto vv1 = sampler(v1);

			auto t = turn(vv, vv0, vv1);
			if (t == Turn::Left)
				return true;
			if (t == Turn::Right)
				return false;
			if (std::abs(vv0.x - vv1.x) > eps)
				return vv0.x < vv1.x;
			return std::abs(vv0.y - vv.y) < std::abs(vv1.y - vv.y);
		};

		auto same = [&](auto v0, auto v1){ return equal(sampler(v0), sampler(v1), eps); };

		auto left = std::min_element(vecs.begin(), vecs.end(), leftmost);
		std::iter_swap(left, vecs.begin());
		auto right = std::min_element(vecs.begin() + 1, vecs.end(), rightmost);
		std::iter_swap(right, vecs.begin() + 1);
		auto top = std::min_element(vecs.begin() + 2, vecs.end(), topmost);
		std::iter_swap(top, vecs.begin() + 2);
		auto bottom = std::min_element(vecs.begin() + 3, vecs.end(), bottommost);
		std::iter_swap(bottom, vecs.begin() + 3);

		refine(vecs.begin() + 4, vecs.end(), vecs[0], vecs[1], vecs[2]);
		refine(vecs.begin() + 4, vecs.end(), vecs[0], vecs[1], vecs[3]);

		std::sort(vecs.begin() + 1, vecs.end(), pred);

		vecs.erase(std::unique(vecs.begin(), vecs.end(), same), vecs.end());

		if (vecs.size() <= 3)
			return vecs;

		std::vector<handle_t> res;
		for (u32 i = 0; i < vecs.size(); i++)
		{
			auto vv = sampler(vecs[i]);
			while (res.size() > 1)
			{
				auto v0 = sampler(*(res.end() - 2));
				auto v1 = sampler(*(res.end() - 1));
				if (turn(v0, v1, vv) == Turn::Left)
					break;
				res.pop_back();
			}
			res.push_back(vecs[i]);
		}
		return res;
	}
}
