#include "primitive_test.h"

#include "primitive.h"

#include <vector>
#include <iostream>

// TODO : add variaty to tests, add reversed lines (v0, v1) -> (v1, v0)

struct LineTest
{
	prim::Line2 l0;
	prim::Line2 l1;
	prim::Status status;
};

bool test_intersection_seg_seg(const LineTest& test)
{
	prim::vec2 p0;
	prim::vec2 p1;

	auto status = prim::intersectSegSeg(test.l0, test.l1, p0, p1);

	return status == test.status;
}

bool test_intersection_seg_line(const LineTest& test)
{
	prim::vec2 p0;

	auto status = prim::intersectSegLine(test.l0, test.l1, p0);

	return status == test.status;
}

void test_intersections_seg_seg()
{
	std::vector<LineTest> noIntersection
	{
		{{{0.0, 0.0}, {0.0, 1.0}}, {{-1.0, 0.0}, {-1.0, 1.0}}, prim::Status::NoIntersection},
		{{{2.0, 0.0}, {2.0, 2.0 - 2e-6}}, {{1.0, 2.0}, {3.0, 2.0}}, prim::Status::NoIntersection},
		{{{4.0, 5.0}, {3.0, 3.0}}, {{4.0, 3.0}, {6.0, 4.0}}, prim::Status::NoIntersection},
		{{{4.0, 0.0}, {4.0, 2.0 - 2e-6}}, {{6.0, 2.0}, {3.5, 2.0}}, prim::Status::NoIntersection},
		{{{10.0, 4.0}, {7.0, 3.0}}, {{9.0, 6.0}, {11.0, 4.0}}, prim::Status::NoIntersection},
		{{{-4.0, 3.0}, {-1.0, 4.0}}, {{-4.0, 4.0}, {-1.0, 5.0}}, prim::Status::NoIntersection},
		//{{{}, {}}, {{}, {}}, prim::Status::NoIntersection},
	};

	std::vector<LineTest> intersection
	{
		{{{1.0, 2.0}, {-1.0, -2.0}}, {{-1.0, 2.0}, {1.0, -2.0}}, prim::Status::Intersection},
		{{{2.0, 2.0}, {3.0, 4.0}}, {{3.0, 4.0}, {4.0, 2.0}}, prim::Status::Intersection},
		{{{2.0, 0.0}, {4.0, 1.0}}, {{3.0, -1.0}, {3.0, 1.0}}, prim::Status::Intersection},
		{{{6.0, 2.0}, {6.0, 4.0}}, {{5.0, 4.0}, {7.0, 4.0}}, prim::Status::Intersection},
		{{{4.0, -1.0}, {6.0, 0.0}}, {{4.0, -1.0-2e-6}, {6.0, 1e-6}}, prim::Status::Intersection},
		{{{1.0, 5.0}, {-1.0, 5.0}}, {{-1.0, 5.0}, {1.0, 6.0}}, prim::Status::Intersection},
		//{{{}, {}}, {{}, {}}, prim::Status::Intersection},
	};

	std::vector<LineTest> overlap
	{
		{{{-2.0, 0.0}, {0.0, 0.0}}, {{2.0, 0.0}, {0.0, 0.0}}, prim::Status::Overlap},
		{{{2.0, 2.0}, {4.0, 4.0}}, {{3.0, 3.0}, {5.0, 5.0}}, prim::Status::Overlap},
		{{{-5.0, 4.0}, {-3.0, 4.0}}, {{-4.5, 4.0}, {-2.5, 4.0}}, prim::Status::Overlap},
		{{{-3.0, -2.0}, {2.0, -1.0}}, {{-0.5, -1.5}, {4.5, -0.5}}, prim::Status::Overlap},
		//{{{}, {}}, {{}, {}}, prim::Status::Overlap},
		//{{{}, {}}, {{}, {}}, prim::Status::Overlap},
		//{{{}, {}}, {{}, {}}, prim::Status::Overlap},
	};

	std::cout << "***********************************" << std::endl;
	std::cout << "**** Non intersecting seg-seg  ****" << std::endl;
	std::cout << "***********************************" << std::endl;
	for (auto& test : noIntersection)
	{
		if (test_intersection_seg_seg(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***********************************" << std::endl;
	std::cout << "**** Intersecting seg-seg      ****" << std::endl;
	std::cout << "***********************************" << std::endl;
	for (auto& test : intersection)
	{
		if (test_intersection_seg_seg(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***********************************" << std::endl;
	std::cout << "**** Overlapping seg-seg       ****" << std::endl;
	std::cout << "***********************************" << std::endl;
	for (auto& test : overlap)
	{
		if (test_intersection_seg_seg(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;
}

void test_intersections_seg_line()
{
	std::vector<LineTest> noIntersection
	{
		{{{0.0, 0.0}, {0.0, 1.0}}, {{-1.0, 0.0}, {-1.0, 1.0}}, prim::Status::NoIntersection},
		{{{-4.0, 3.0}, {-1.0, 4.0}}, {{-4.0, 4.0}, {-1.0, 5.0}}, prim::Status::NoIntersection},
		{{{1.0, 3.0}, {3.0, 1.0}}, {{2.0, 0.0}, {4.0, -2.0}}, prim::Status::NoIntersection},
		{{{3.0, 1.0}, {1.0, 3.0}}, {{2.0, 0.0}, {4.0, -2.0}}, prim::Status::NoIntersection},
	};

	std::vector<LineTest> intersection
	{
		{{{1.0, 2.0}, {-1.0, -2.0}}, {{-1.0, 2.0}, {1.0, -2.0}}, prim::Status::Intersection},
		{{{2.0, 2.0}, {3.0, 4.0}}, {{3.0, 4.0}, {4.0, 2.0}}, prim::Status::Intersection},
		{{{2.0, 0.0}, {4.0, 1.0}}, {{3.0, -1.0}, {3.0, 1.0}}, prim::Status::Intersection},
		{{{6.0, 2.0}, {6.0, 4.0}}, {{5.0, 4.0}, {7.0, 4.0}}, prim::Status::Intersection},
		{{{4.0, -1.0}, {6.0, 0.0}}, {{4.0, -1.0-2e-6}, {6.0, 1e-6}}, prim::Status::Intersection},
		{{{1.0, 5.0}, {-1.0, 5.0}}, {{-1.0, 5.0}, {1.0, 6.0}}, prim::Status::Intersection},
	};

	std::vector<LineTest> overlap
	{
		{{{-2.0, 0.0}, {0.0, 0.0}}, {{2.0, 0.0}, {0.0, 0.0}}, prim::Status::Overlap},
		{{{2.0, 2.0}, {4.0, 4.0}}, {{3.0, 3.0}, {5.0, 5.0}}, prim::Status::Overlap},
		{{{-5.0, 4.0}, {-3.0, 4.0}}, {{-4.5, 4.0}, {-2.5, 4.0}}, prim::Status::Overlap},
		{{{-3.0, -2.0}, {2.0, -1.0}}, {{-0.5, -1.5}, {4.5, -0.5}}, prim::Status::Overlap},
	};

	std::cout << "************************************" << std::endl;
	std::cout << "**** Non intersecting seg-line  ****" << std::endl;
	std::cout << "************************************" << std::endl;
	for (auto& test : noIntersection)
	{
		if (test_intersection_seg_line(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "************************************" << std::endl;
	std::cout << "**** Intersecting seg-line      ****" << std::endl;
	std::cout << "************************************" << std::endl;
	for (auto& test : intersection)
	{
		if (test_intersection_seg_line(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "************************************" << std::endl;
	std::cout << "**** Overlapping seg-line       ****" << std::endl;
	std::cout << "************************************" << std::endl;
	for (auto& test : overlap)
	{
		if (test_intersection_seg_line(test))
			std::cout << "Passed. " << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;
}


struct LineTestP
{
	prim::Line2 line;
	prim::Float p;
	prim::Status status;
};

bool test_intersectionX(const LineTestP& test)
{
	prim::Float v;
	auto status = prim::intersectsSegX(test.line, test.p, v);

	return status == test.status;
}

bool test_intersectionY(const LineTestP& test)
{
	prim::Float v;
	auto status = prim::intersectsSegY(test.line, test.p, v);

	return status == test.status;
}

void test_intersectionsX()
{
	std::vector<LineTestP> intersection
	{
		{{{1.0, 3.0}, {3.0, 5.0}}, 2.0, prim::Status::Intersection},
		{{{1.0, 5.0}, {3.0, 3.0}}, 2.0, prim::Status::Intersection},
		{{{3.0, -3.0}, {6.0, 3.0}}, 5.0, prim::Status::Intersection},
		{{{5.0 - 1e-6, -1.0}, {5.0 + 1e-6, -3.0}}, 5.0, prim::Status::Intersection},
		{{{5.0 - 1e-6, 5.0}, {5.0 + 1e-6, 3.0}}, 5.0, prim::Status::Intersection},
		{{{4.0, 2.0}, {5.0, 2.0}}, 5.0, prim::Status::Intersection},
		{{{5.0, 2.0}, {6.0, 2.0}}, 5.0, prim::Status::Intersection},
	};

	std::vector<LineTestP> noIntersection
	{
		{{{2.0 - 1.5e-6, -1.0}, {2.0 - 1.5e-6, -3.0}}, 2.0, prim::Status::NoIntersection},
		{{{2.0 + 1.5e-6, -1.0}, {2.0 + 1.5e-6, 1.0}}, 2.0, prim::Status::NoIntersection},
		{{{-1.0, 0.0}, {1.0, 2.0}}, 2.0, prim::Status::NoIntersection},
		{{{0.0, 5.0}, {2.0 - 1.1e-6, 3.0}}, 2.0, prim::Status::NoIntersection},
		{{{2.0 + 1.1e-6, 3.0}, {4.0, 1.0}}, 2.0, prim::Status::NoIntersection},
		{{{3.0, 3.0}, {5.0, 5.0}}, 2.0, prim::Status::NoIntersection},
	};

	std::vector<LineTestP> overlap
	{
		{{{2.0, 0.0}, {2.0, -2.0}}, 2.0, prim::Status::Overlap},
		{{{2.0, 1.0}, {2.0, 3.0}}, 2.0, prim::Status::Overlap},
		{{{3.0, 4.0}, {3.0, 2.0}}, 3.0, prim::Status::Overlap},
		{{{3.0, 3.0}, {3.0, 1.0}}, 3.0, prim::Status::Overlap},
		{{{3.0, 2.0}, {3.0, -1.0}}, 3.0, prim::Status::Overlap},
		{{{3.0, 0.0}, {3.0, -2.0}}, 3.0, prim::Status::Overlap},
	};

	std::cout << "**************************" << std::endl;
	std::cout << "**** intersection X   ****" << std::endl;
	std::cout << "**************************" << std::endl;
	for (auto& test : intersection)
	{
		if (test_intersectionX(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***************************" << std::endl;
	std::cout << "**** no intersection X ****" << std::endl;
	std::cout << "***************************" << std::endl;
	for (auto& test : noIntersection)
	{
		if (test_intersectionX(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***************************" << std::endl;
	std::cout << "**** overlap X         ****" << std::endl;
	std::cout << "***************************" << std::endl;
	for (auto& test : overlap)
	{
		if (test_intersectionX(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;
}

void test_intersectionsY()
{
	std::vector<LineTestP> intersection
	{
		{{{3.0, 1.0}, {5.0, 3.0}}, 2.0, prim::Status::Intersection},
		{{{5.0, 1.0}, {3.0, 3.0}}, 2.0, prim::Status::Intersection},
		{{{-3.0, 3.0}, {3.0, 6.0}}, 5.0, prim::Status::Intersection},
		{{{-1.0, 5.0 - 1e-6}, {-3.0, 5.0 + 1e-6}}, 5.0, prim::Status::Intersection},
		{{{5.0, 5.0 - 1e-6}, {3.0, 5.0 + 1e-6}}, 5.0, prim::Status::Intersection},
		{{{2.0, 4.0}, {2.0, 5.0}}, 5.0, prim::Status::Intersection},
		{{{2.0, 5.0}, {2.0, 6.0}}, 5.0, prim::Status::Intersection},
	};

	std::vector<LineTestP> noIntersection
	{
		{{{-1.0, 2.0 - 1.5e-6}, {-3.0, 2.0 - 1.5e-6}}, 2.0, prim::Status::NoIntersection},
		{{{-1.0, 2.0 + 1.5e-6}, {1.0, 2.0 + 1.5e-6}}, 2.0, prim::Status::NoIntersection},
		{{{0.0, -1.0}, {2.0, 1.0}}, 2.0, prim::Status::NoIntersection},
		{{{5.0, 0.0}, {3.0, 2.0 - 1.1e-6}}, 2.0, prim::Status::NoIntersection},
		{{{3.0, 2.0 + 1.1e-6}, {1.0, 4.0}}, 2.0, prim::Status::NoIntersection},
		{{{3.0, 3.0}, {5.0, 5.0}}, 2.0, prim::Status::NoIntersection},
	};

	std::vector<LineTestP> overlap
	{
		{{{0.0, 2.0}, {-2.0, 2.0}}, 2.0, prim::Status::Overlap},
		{{{1.0, 2.0}, {3.0, 2.0}}, 2.0, prim::Status::Overlap},
		{{{4.0, 3.0}, {2.0, 3.0}}, 3.0, prim::Status::Overlap},
		{{{3.0, 3.0}, {1.0, 3.0}}, 3.0, prim::Status::Overlap},
		{{{2.0, 3.0}, {-1.0, 3.0}}, 3.0, prim::Status::Overlap},
		{{{0.0, 3.0}, {-2.0, 3.0}}, 3.0, prim::Status::Overlap},
	};

	std::cout << "**************************" << std::endl;
	std::cout << "**** intersection Y   ****" << std::endl;
	std::cout << "**************************" << std::endl;
	for (auto& test : intersection)
	{
		if (test_intersectionY(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***************************" << std::endl;
	std::cout << "**** no intersection Y ****" << std::endl;
	std::cout << "***************************" << std::endl;
	for (auto& test : noIntersection)
	{
		if (test_intersectionY(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;

	std::cout << "***************************" << std::endl;
	std::cout << "**** overlap Y         ****" << std::endl;
	std::cout << "***************************" << std::endl;
	for (auto& test : overlap)
	{
		if (test_intersectionY(test))
			std::cout << "Passed." << std::endl;
		else
			std::cout << "Failed." << std::endl;
	}
	std::cout << std::endl;
}