#pragma once

#include "primitive.h"

#include <vector>

namespace sect
{
	using vec2  = prim::vec2;
	using Line2 = prim::Line2;

	struct Event
	{
		std::vector<Line2*> lowerPoint;
		std::vector<Line2*> upperPoint;

		vec2 pt{};
	};
}
