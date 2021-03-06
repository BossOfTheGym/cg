#include "section_segments_test.h"
#include "section.h"

#include <iostream>
#include <numeric>

namespace
{
	std::ostream& operator << (std::ostream& out, const prim::vec2& p)
	{
		return out << "(" <<  p.x << " " << p.y << ")";	
	}
	
	std::ostream& operator << (std::ostream& out, const prim::Line2& l)
	{
		return out << l.v0 << " " << l.v1;
	}
}

void test_intersections(const std::vector<prim::Line2>& lines)
{
	std::vector<u32> handles(lines.size());
	std::iota(handles.begin(), handles.end(), 0u);

	auto sampler = [&] (u32 handle)
	{
		return lines[handle];
	};

	auto intersections = sect::section_n_lines<u32>(handles, sampler);

	if (intersections.size() > 0)
	{
		std::cout << "intersections: " << intersections.size() << std::endl;
		for (auto& [p, segs] : intersections)
		{
			std::cout << "point " << p << std::endl;		
			for (auto& seg : segs)
				std::cout << "    line " << sampler(seg) << std::endl;
		}
	}
	else
	{
		std::cout << "No intersections" << std::endl;
	}
}

void test_intersections()
{
	std::vector<std::vector<prim::Line2>> tests
	{
		// 1(6 inters)
		{
			{{3.0, 4.0}, {3.0, 9.0}},
		{{2.0, 8.0}, {4.0, 4.0}},
		{{1.0, 5.0}, {5.0, 7.0}},
		{{4.0, 7.0}, {7.0, 4.0}},
		{{1.0, 2.0}, {6.0, 7.0}},
		{{7.0, 6.0}, {5.0, 4.0}},
		{{6.0, 5.0}, {5.0, 2.0}},
		{{6.0, 5.0}, {7.0, 2.0}},
		},

		// 2(1 inter)
		{
			{{1.0, 3.0}, {3.0, 7.0}},
		{{2.0, 3.0}, {2.0, 7.0}},
		{{3.0, 3.0}, {1.0, 7.0}},
		{{4.0, 6.0}, {5.0, 3.0}},
		{{6.0, 3.0}, {7.0, 7.0}},
		{{1.0, 6.0}, {3.0, 4.0}},
		{{1.0, 1.0}, {3.0, 9.0}},
		{{-1.0, 0.0}, {-1.0, 7.0}},
		},

		// 3(4 inters)
		{
			{{3.0, 8.0}, {7.0, 4.0}},
		{{3.0, 6.0}, {7.0, 2.0}},
		{{3.0, 4.0}, {7.0, 8.0}},
		{{3.0, 2.0}, {7.0, 6.0}},
		{{5.0, 8.0}, {5.0, 2.0}},
		},

		// 4(23 inters)
		{
			{{1.0, 4.0}, {3.0, 2.0}},
		{{5.0, 2.0}, {1.0, 6.0}},
		{{7.0, 2.0}, {1.0, 8.0}},
		{{9.0, 2.0}, {1.0, 10.0}},
		{{9.0, 4.0}, {3.0, 10.0}},
		{{9.0, 6.0}, {5.0, 10.0}},
		{{9.0, 8.0}, {7.0, 10.0}},
		{{4.0, 9.0}, {2.0, 7.0}},
		{{6.0, 9.0}, {2.0, 5.0}},
		{{8.0, 9.0}, {2.0, 3.0}},
		{{8.0, 7.0}, {4.0, 3.0}},
		{{6.0, 3.0}, {8.0, 5.0}},
		},

		// 5(no inters)
		{
			{{1.0, 8.0}, {4.0, 10.0}},
		{{1.0, 6.0}, {4.0, 8.0}},
		{{2.0, 5.0}, {5.0, 7.0}},
		{{1.0, 3.0}, {4.0, 5.0}},
		{{5.0, 10.0}, {8.0, 8.0}},
		{{5.0, 8.0}, {8.0, 6.0}},
		{{5.0, 6.0}, {8.0, 5.0}},
		{{5.0, 5.0}, {8.0, 4.0}},
		{{4.0, 4.0}, {7.0, 3.0}},
		{{5.0, 3.0}, {8.0, 2.0}},
		{{8.0, 9.0}, {11.0, 11.0}},
		{{8.0, 7.0}, {11.0, 9.0}},
		{{8.0, 3.0}, {11.0, 5.0}},
		{{8.0, 1.0}, {11.0, 3.0}},
		{{11.0, 10.0}, {14.0, 8.0}},
		{{11.0, 8.0}, {14.0, 6.0}},
		{{11.0, 6.0}, {14.0, 4.0}},
		{{11.0, 4.0}, {14.0, 2.0}},
		},
	};

	std::cout << "***********************************" << std::endl;
	std::cout << "**** Simple intersection tests ****" << std::endl;
	std::cout << "***********************************" << std::endl;

	u32 num = 1;
	for (auto& test : tests)
	{
		std::cout << "******** Test " << num << " ********" << std::endl;
		test_intersections(test);

		++num;

		std::cout << std::endl;
	}
}

void test_intersections_with_horiz()
{
	std::vector<std::vector<prim::Line2>> tests
	{
		// 6 intersections
		{
			{{1.0, 8.0}, {5.0, 2.0}},
		{{3.0, 9.0}, {3.0, 1.0}},
		{{5.0, 8.0}, {1.0, 2.0}},
		{{4.0, 8.0}, {8.0, 2.0}},
		{{6.0, 9.0}, {6.0, 1.0}},
		{{8.0, 8.0}, {4.0, 2.0}},
		{{1.0, 5.0}, {3.0, 5.0}},
		{{6.0, 5.0}, {3.0, 5.0}},
		{{8.0, 5.0}, {6.0, 5.0}},
		{{8.0, 5.0}, {1.0, 5.0}},
		},

		// 5 intersections
		{
			{{3.0, 1.0}, {3.0, 5.0}},
		{{1.0, 3.0}, {5.0, 3.0}},
		{{3.0, 5.0}, {1.0, 3.0}},
		{{3.0, 5.0}, {5.0, 3.0}},
		{{3.0, 1.0}, {1.0, 3.0}},
		{{5.0, 3.0}, {3.0, 1.0}},
		},
	};

	std::cout << "********************************************" << std::endl;
	std::cout << "**** Intersection tests with horizontal ****" << std::endl;
	std::cout << "********************************************" << std::endl;

	u32 num = 1;
	for (auto& test : tests)
	{
		std::cout << "******** Test " << num << " ********" << std::endl;
		test_intersections(test);

		++num;

		std::cout << std::endl;
	}
}
