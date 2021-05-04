#pragma once

#include "core.h"
#include "primitive.h"

#include <vector>
#include <cassert>
#include <utility>
#include <algorithm>

namespace tria
{
	using namespace prim;

	// TODO : rename, remove tria from name(I have namespace already)
	template<class handle_t, class sampler_t>
	std::vector<handle_t> tria_graham(std::vector<handle_t>& vecs, sampler_t sampler, Float eps = default_eps)
	{
		if (vecs.size() < 3)
			return {};

		auto leftmost = [&](auto v0, auto v1)
		{
			auto vv0 = sampler(v0);
			auto vv1 = sampler(v1);
			if (std::abs(vv0.x - vv1.x) > eps)
				return vv0.x < vv1.x;
			return std::abs(vv0.y - vv1.y) > eps && vv0.y < vv1.y;
		};
		
		auto pred = [&](auto v0, auto v1)
		{
			auto vv  = sampler(vecs[0]);
			auto vv0 = sampler(v0);
			auto vv1 = sampler(v1);

			auto t = turn_exact(vv, vv0, vv1);
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
		
		std::sort(vecs.begin() + 1, vecs.end(), pred);

		vecs.erase(std::unique(vecs.begin(), vecs.end(), same), vecs.end());

		if (vecs.size() < 3)
			return {};

		std::vector<handle_t> tri;
		tri.push_back(vecs[0]);
		tri.push_back(vecs[1]);
		std::vector<handle_t> res;
		res.push_back(vecs[0]);
		res.push_back(vecs[1]);
		for (u32 i = 2; i < vecs.size(); i++)
		{
			tri.push_back(res.back());
			tri.push_back(vecs[i]);
			if (turn_exact(sampler(vecs[0]), sampler(res.back()), sampler(vecs[i])) != Turn::Straight)
			{
				tri.push_back(vecs[0]);
				tri.push_back(vecs[i]);
			}

			auto vv = sampler(vecs[i]);
			while (res.size() > 1)
			{
				auto v0 = sampler(*(res.end() - 2));
				auto v1 = sampler(*(res.end() - 1));
				auto t = turn_exact(v0, v1, vv);
				if (t == Turn::Left)
					break;
				if (t == Turn::Right)
				{
					tri.push_back(*(res.end() - 2));
					tri.push_back(vecs[i]);
				}
				res.pop_back();
			}
			res.push_back(vecs[i]);
		}
		return tri;
	}
}
