#pragma once

#include "core.h"
#include "primitive.h"

#include <utility>
#include <algorithm>
#include <vector>

namespace ds
{
	using prim::vec2;
	using handle = u32;

	constexpr handle null = std::numeric_limits<handle>::max();

	struct Vertex;
	struct HalfEdge;
	struct Face;

	struct Vertex
	{
		vec2 pos{};
		handle edge{null};
	};

	struct HalfEdge
	{
		handle prevEdge{null};
		handle nextEdge{null};
		handle twinEdge{null};
		handle vertex{null};
		handle face{null};
	};

	struct Face
	{
		handle inner{};
	};

	struct Bundle
	{
		std::vector<Vertex>   vertices{};
		std::vector<HalfEdge> edges{};
		std::vector<Face>     faces{};
	};
}
