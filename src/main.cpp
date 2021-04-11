#include "section.h"

#include <iostream>

std::ostream& operator << (std::ostream& out, const prim::vec2& p)
{
	return out << "(" <<  p.x << " " << p.y << ")";	
}

std::ostream& operator << (std::ostream& out, const prim::Line2& l)
{
	return out << l.v0 << " " << l.v1;
}

void test_intersections(const std::vector<prim::Line2>& lines)
{
	std::cout << "TEST" << std::endl;

	auto intersections = sect::section_n_lines(lines);
	for (auto& [p, segs] : intersections)
	{
		std::cout << "point " << p << std::endl;		
		for (auto& seg : segs)
			std::cout << "    line " << seg << std::endl;
	}

	std::cout << std::endl;
}


void test_intersections()
{
	std::vector<std::vector<prim::Line2>> tests
	{
		//{
		//	{{3.0, 4.0}, {3.0, 9.0}},
		//	{{2.0, 8.0}, {4.0, 4.0}},
		//	{{1.0, 5.0}, {5.0, 7.0}},
		//	{{4.0, 7.0}, {7.0, 4.0}},
		//	{{1.0, 2.0}, {6.0, 7.0}},
		//	{{7.0, 6.0}, {5.0, 4.0}},
		//	{{6.0, 5.0}, {5.0, 2.0}},
		//	{{6.0, 5.0}, {7.0, 2.0}},
		//},
		{
			{{1.0, 3.0}, {3.0, 7.0}},
			{{2.0, 3.0}, {2.0, 7.0}},
			{{3.0, 3.0}, {1.0, 7.0}},
			{{4.0, 6.0}, {5.0, 3.0}},
			{{6.0, 3.0}, {7.0, 7.0}},
			{{1.0, 6.0}, {3.0, 4.0}},
			{{1.0, 1.0}, {3.0, 9.0}},
		}
	};

	for (auto& test : tests)
		test_intersections(test);
}

int main()
{
	test_intersections();

	return 0;
}