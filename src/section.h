#pragma once

#include "primitive.h"

#include <vector>

namespace sect
{
	struct Intersection
	{
		// intersection point
		prim::vec2 point;

		// set of intersecting segments
		std::vector<prim::Line2> lines;
	};

	std::vector<Intersection> section_n_lines(std::vector<prim::Line2>& lines);
}
