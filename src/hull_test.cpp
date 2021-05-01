#include "hull_test.h"

#include "convex_hull.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <numeric>

using namespace prim;

namespace
{
	struct Test
	{
		std::vector<Vec2> vecs;
		std::vector<u32> answer;
	};

	std::ostream& operator << (std::ostream& out, const Vec2& p)
	{
		return out << "(" <<  p.x << " " << p.y << ")";	
	}

	std::ostream& operator << (std::ostream& out, const Line2& l)
	{
		return out << l.v0 << l.v1;
	}
}

void test_hull(const Test& test)
{
	auto sampler = [&](u32 handle)
	{
		assert(handle < test.vecs.size());

		return test.vecs[handle];
	};

	std::vector<u32> handles(test.vecs.size());
	std::iota(handles.begin(), handles.end(), 0u);

	//std::cout << "ans: ";
	//for (auto& handle : test.answer)
	//	std::cout << sampler(handle) << " ";
	//std::cout << std::endl;

	auto res = hull::convex_hull_graham(handles, sampler);
	std::cout << "got: ";
	for (auto& handle : res)
		std::cout << sampler(handle) << " ";
	std::cout << std::endl;
}

void test_hull()
{
	std::vector<Test> tests = 
	{
		Test
		{
			{
				{1.0, 1.0},
				{1.0, 2.0},
				{1.0, 3.0},
				{1.0, 4.0},
				{1.0, 5.0},
				{2.0, 2.0},
				{2.0, 4.0},
				{3.0, 1.0},
				{3.0, 3.0},
				{3.0, 5.0},
				{4.0, 0.0},
				{4.0, 2.0},
				{4.0, 4.0},
				{4.0, 6.0},
				{5.0, 0.0},
				{5.0, 2.0},
				{5.0, 4.0},
				{5.0, 6.0},
				{6.0, 0.0},
				{6.0, 3.0},
				{6.0, 6.0},
				{7.0, 1.0},
				{7.0, 2.0},
				{7.0, 4.0},
				{7.0, 5.0},
			},
			{}
		},
	};

	std::cout << "*******************" << std::endl;
	std::cout << "**** Hull test ****" << std::endl;
	std::cout << "*******************" << std::endl;

	u32 curr = 0;
	for (auto& test : tests)
	{
		std::cout << "test " << curr << std::endl;
		test_hull(test);
		std::cout << std::endl;

		++curr;
	}
}
