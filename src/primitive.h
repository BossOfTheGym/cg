#pragma once

#include "core.h"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace prim
{
	// points v0, v1, v2 are listed in counterclockwise order
	using namespace glm;


	void init();

	using Float = f64;

	using Vec2 = dvec2;
	using Vec3 = dvec3;

	struct Line2
	{
		Vec2 v0{};
		Vec2 v1{};
	};

	struct AABB2
	{
		Vec2 v0{};
		Vec2 v1{};
	};

	struct Triangle2
	{
		static Triangle2 construct(const Vec2& v0, const Vec2& v1, const Vec2& v2);

		Vec2 v0{};
		Vec2 v1{};
		Vec2 v2{};
	};

	struct Circle3V2
	{
		Vec2 v0{};
		Vec2 v1{};
		Vec2 v2{};
	};

	struct CircleRV2
	{
		Vec2  c{};
		Float r{};
	};

	// abs() <= eps => equal
	// abs() > eps => not equal
	static constexpr Float default_eps = 1e-10;

	// math
	// computes 2x2 determinant (ad - bc)
	// | a b |
	// | c d |
	Float det4(Float a, Float b, Float c, Float d);

	Float dot2(const Vec2& v0);

	Float cross_z(const Vec2& v0, const Vec2& v1);
	

	// orientation
	bool inAABB(const AABB2& aabb, const Vec2& vec);

	bool inAABB(const AABB2& tested, const AABB2& aabb);

	bool inTriangle(const Triangle2& tri, const Vec2& vec);

	bool inTriangle(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& v);

	bool overlaps(const AABB2& a, const AABB2& b);

	bool overlaps(Float a, Float b, Float c, Float d);

	bool overlaps_checked(Float a, Float b, Float c, Float d);

	bool equal(const Vec2& v0, const Vec2& v1, Float eps = default_eps);

	bool either_end(const Vec2& v, const Line2& line, Float eps = default_eps);

	bool belongs(const Vec2& v, const Line2& l, Float eps = default_eps);

	bool horiz(const Line2& l, Float eps = default_eps);

	bool vert(const Line2& l, Float eps = default_eps);


	// intersection
	enum class Status : i32
	{
		NoIntersection,
		Intersection,
		Overlap,
	};

	Status intersectSegSeg(const Line2& s0, const Line2& s1, Vec2& v0, Vec2& v1, Float eps = default_eps);

	Status intersectSegLine(const Line2& s, const Line2& l, Vec2& v, Float eps = default_eps);

	Status intersectLineLine(const Line2& l0, const Line2& l1, Vec2& v, Float eps = default_eps);

	Status intersectsSegX(const Line2& s, Float x, Float& yi, Float eps = default_eps);

	Status intersectsSegY(const Line2& s, Float y, Float& xi, Float eps = default_eps);

	Status intersectsLineX(const Line2& l, Float x, Float& yi, Float eps = default_eps);

	Status intersectsLineY(const Line2& l, Float y, Float& xi, Float eps = default_eps);


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

	Turn turn(const Vec2& v0, const Vec2& v1, const Vec2& v2, Float eps = default_eps);

	Turn turn_exact(const Vec2& v0, const Vec2& v1, const Vec2& v2);

	Orient circleOrient(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& v3, Float eps = default_eps);

	Orient circleOrient_exact(const Vec2& v0, const Vec2& v1, const Vec2& v2, const Vec2& v3);
}
