#pragma once

#include "core.h"
#include "primitive.h"

#include <utility>
#include <algorithm>
#include <vector>


// just general description
// TODO : this is little bit primitive representation of dcel
// TODO : maybe go full adjacency?
namespace ds
{
	using vec2 = prim::vec2;

	struct Vertex;
	struct HalfEdge;
	struct Face;

	// TODO : maybe go full adjacency?
	struct Vertex
	{
		vec2* pos{};
		HalfEdge* edge{};
	};

	struct HalfEdge
	{
		HalfEdge* prev{};
		HalfEdge* next{};
		HalfEdge* twin{};
		Vertex* v0{};
		Face* face{};
	};

	// TODO : maybe go full adjacency?
	struct Face
	{
		HalfEdge* inner{};
		HalfEdge* outer{};
	};

	struct Bundle
	{
		std::vector<vec2>     vertexData{};
		std::vector<Vertex>   vertices{};
		std::vector<HalfEdge> halfEdges{};
		std::vector<Face>     faces{};
	};
}
