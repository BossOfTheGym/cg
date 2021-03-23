#pragma once

#include "core.h"

#include <cmath>

#include <glm/glm.hpp>

namespace prim
{
	// points v0, v1, v2 are listed in counterclockwise order

	using namespace glm;

	using Float = f32;


	struct Line2
	{
		vec2 v0{};
		vec2 v1{};
	};

	struct AABB2
	{
		vec2 v0{};
		vec2 v1{};
	};

	struct Triangle2
	{
		vec2 v0{};
		vec2 v1{};
		vec2 v2{};
	};

	struct Circle3V2
	{
		vec2 v0{};
		vec2 v1{};
		vec2 v2{};
	};

	struct CircleRV2
	{
		vec2  c{};
		Float r{};
	};


	// math
	Float dot2(const vec2& v0);

	Float cross_z(const vec2& v0, const vec2& v1);

	Float len(const vec2& v);
	

	// orientation
	bool inAABB(const AABB2& aabb, const vec2& vec);

	bool inTriangle(const Triangle2& tri, const vec2& vec);

	bool overlaps(const AABB2& a, const AABB2& b);

	bool overlaps(Float a, Float b, Float c, Float d);

	bool overlaps_checked(Float a, Float b, Float c, Float d);


	// intersection
	enum class Status : i32
	{
		NoIntersection,
		Intersection,
		Overlap,
	};

	// TODO : test
	Status intersectSegSeg(const Line2& s0, const Line2& s1, vec2& v0, vec2& v1, Float eps = 1e-6);

	// TODO : test
	Status intersectSegLine(const Line2& seg, const Line2& line, vec2& v, Float eps = 1e-6);

	// TODO : test
	Status intersectsSegX(const Line2& seg, Float x, vec2& v, Float eps = 1e-6);

	// TODO : test
	Status intersectsSegY(const Line2& seg, Float y, vec2& v, Float eps = 1e-6);


	// predicates
	enum class Turn : i32
	{
		Left = -1,
		Straight = 0,
		Right = 1,
	};

	enum class Orient : i32
	{
		In = -1,
		On = 0,
		Out = 1,
	};


	Turn turn(const vec2& v0, const vec2& v1, const vec2& v2, Float eps = 1e-6);

	bool leftTurn(const vec2& v0, const vec2& v1, const vec2& v2, Float eps = 1e-6);

	Orient circleOrient(const vec2& v0, const vec2& v1, const vec2& v2, const vec2& v3, Float eps = 1e-6);
}
