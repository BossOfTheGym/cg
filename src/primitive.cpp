#include "primitive.h"

#include <algorithm>

namespace prim
{
	// math
	Float det4(Float a, Float b, Float c, Float d)
	{
		return a * d - b * c;
	}

	Float dot2(const vec2& v0)
	{
		return dot(v0, v0);
	}

	Float cross_z(const vec2& v0, const vec2& v1)
	{
		return v0.x * v1.y - v0.y * v1.x;
	}

	Float len(const vec2& v)
	{
		return std::sqrt(dot2(v));
	}


	// orientation
	bool inAABB(const AABB2& aabb, const vec2& vec)
	{
		return aabb.v0.x <= vec.x && vec.x <= aabb.v1.x
			&& aabb.v0.y <= vec.y && vec.y <= aabb.v1.y;
	}

	bool inTriangle(const Triangle2& tri, const vec2& v)
	{
		return cross_z(tri.v1 - tri.v0, v - tri.v1) >= 0
			&& cross_z(tri.v2 - tri.v1, v - tri.v2) >= 0
			&& cross_z(tri.v0 - tri.v2, v - tri.v0) >= 0;
	}

	bool overlaps(const AABB2& a, const AABB2& b)
	{
		return std::max(a.v0.x, b.v0.x) <= std::min(a.v1.x, b.v1.x)
			&& std::max(a.v0.y, b.v0.y) <= std::min(a.v1.y, b.v1.y);
		//return a.v1.x >= b.v0.x && a.v0.x <= b.v1.x
		//	&& a.v1.y >= b.v0.y && a.v0.y <= b.v1.y;
	}

	bool overlaps(Float a, Float b, Float c, Float d)
	{
		return std::max(a, c) <= std::min(b, d);
	}

	bool overlaps_checked(Float a, Float b, Float c, Float d)
	{
		if (a > b)
			std::swap(a, b);		
		if (c > d)
			std::swap(c, d);
		return overlaps(a, b, c, d);
	}

	bool equal(const vec2& v0, const vec2& v1, Float eps)
	{
		auto dv = abs(v1 - v0);

		return dv.x <= eps && dv.y <= eps;
	}

	bool either_end(const vec2& v, const Line2& line, Float eps)
	{
		return equal(v, line.v0, eps) || equal(v, line.v1, eps);
	}

	bool belongs(const vec2& v, const Line2& l, Float eps)
	{
		auto dl = l.v1 - l.v0;
		auto dv = v - l.v0;
		auto det = cross_z(dl, dv);
		if (std::abs(det) <= eps) 
		{
			auto u = dot(dl, dv) / dot(dl, dl);
			if (0.0 <= u && u <= 1.0)
				return true;
		}
		return false;
	}


	// intersection
	Status intersectSegSeg(const Line2& s0, const Line2& s1, vec2& v0, vec2& v1, Float eps)
	{
		if (!overlaps_checked(s0.v0.x, s0.v1.x, s1.v0.x, s1.v1.x) || !overlaps_checked(s0.v0.y, s0.v1.y, s1.v0.y, s1.v1.y))
			return Status::NoIntersection;

		auto ds0 = s0.v1 - s0.v0;
		auto ds1 = s1.v1 - s1.v0;

		auto det = -cross_z(ds0, ds1);
		if (std::abs(det) > eps)
		{
			auto ds1s0 = s1.v0 - s0.v0;

			Float u0 = -cross_z(ds1s0, ds1) / det;
			if (u0 < 0 || u0 > 1)
				return Status::NoIntersection;

			Float u1 = cross_z(ds0, ds1s0) / det;
			if (u1 < 0 || u1 > 1)
				return Status::NoIntersection;

			v0 = s0.v0 + ds0 * u0;
			return Status::Intersection;
		}

		if (std::abs(cross_z(ds0, s1.v0 - s0.v0)) > eps)
			return Status::NoIntersection;

		auto ds0ds0 = dot2(ds0);
		auto c = dot(ds0, s1.v0 - s0.v0) / ds0ds0;
		auto d = dot(ds0, s1.v1 - s0.v0) / ds0ds0;
		auto vc = s1.v0;
		auto vd = s1.v1;
		if (c > d)
		{
			std::swap(c, d);
			std::swap(vc, vd);
		}

		if (!overlaps(0.0, 1.0, c, d))
			return Status::NoIntersection;

		if (c > 0)
			v0 = vc;
		else		
			v0 = s0.v0;

		if (d < 1)
			v1 = vd;
		else
			v1 = s0.v1;
		return Status::Overlap;
	}

	Status intersectSegLine(const Line2& seg, const Line2& line, vec2& v, Float eps)
	{
		auto ds = seg.v1 - seg.v0;
		auto dl = line.v1 - line.v0;

		auto det = -cross_z(ds, dl);
		if (std::abs(det) > eps) 
		{
			auto dls = line.v0 - seg.v0;

			Float u = -cross_z(dls, dl) / det;
			if (u < 0 || u > 1)
				return Status::NoIntersection;

			v = seg.v0 + ds * u;
			return Status::Intersection;
		}

		if (std::abs(cross_z(ds, line.v0 - seg.v0)) <= eps)
			return Status::Overlap;

		return Status::NoIntersection;
	}

	Status intersectsSegX(const Line2& seg, Float x, vec2& v, Float eps)
	{
		Float x0 = seg.v0.x;
		Float x1 = seg.v1.x;
		if (x0 > x1)
			std::swap(x0, x1);

		Float dx = x1 - x0;
		if (std::abs(dx) > eps)
		{
			if (x < x0 || x1 < x)
				return Status::NoIntersection;

			Float y0 = seg.v0.y;
			Float y1 = seg.v1.y;
			Float dy = y1 - y0;

			v.x = x;
			v.y = dy / dx * (x - x0) + y0;

			return Status::Intersection;
		}

		if (std::abs(x - x0) > eps)
			return Status::NoIntersection;
		return Status::Overlap;
	}

	Status intersectsSegY(const Line2& seg, Float y, vec2& v, Float eps)
	{
		Float y0 = seg.v0.y;
		Float y1 = seg.v1.y;
		if (y0 > y1)
			std::swap(y0, y1);

		Float dy = y1 - y0;
		if (std::abs(dy) > eps) 
		{
			if (y < y0 || y1 < y)
				return Status::NoIntersection;

			Float x0 = seg.v0.x;
			Float x1 = seg.v1.x;
			Float dx = x1 - x0;

			v.x = dx / dy * (y - y0) + x0;
			v.y = y;

			return Status::Intersection;
		}

		if (std::abs(y - y0) > eps)
			return Status::NoIntersection;
		return Status::Overlap;
	}


	// predicates
	Turn turn(const vec2& v0, const vec2& v1, const vec2& v2, Float eps)
	{
		auto dv1v0 = v1 - v0;
		auto dv2v1 = v2 - v1;

		auto orient = Turn::Straight;
		auto det = cross_z(dv2v1, dv1v0);
		if (std::abs(det) <= eps)
			return Turn::Straight;
		if (det > 0)
			return Turn::Right;
		return Turn::Left;
	}

	bool leftTurn(const vec2& v0, const vec2& v1, const vec2& v2, Float eps)
	{
		return turn(v0, v1, v2, eps) == Turn::Left;
	}

	Orient circleOrient(const vec2& v0, const vec2& v1, const vec2& v2, const vec2& v3, Float eps)
	{
		// |v0_x v0_y v0_x^2 + v0_y^2 1|
		// |v1_x v1_y v1_x^2 + v1_y^2 1|
		// |v2_x v2_y v2_x^2 + v2_y^2 1|
		// |v3_x v3_y v3_x^2 + v3_y^2 1|

		Float v0x = v0.x, v0y = v0.y, v0z = v0.x * v0.x + v0.y * v0.y;
		Float v1x = v0.x, v1y = v0.y, v1z = v1.x * v1.x + v1.y * v1.y;
		Float v2x = v0.x, v2y = v0.y, v2z = v2.x * v2.x + v2.y * v2.y;
		Float v3x = v0.x, v3y = v0.y, v3z = v3.x * v3.x + v3.y * v3.y;

		Float det = v0x * (v1y * v2z - v1z * v2y) - v0y * (v1x * v2z - v1z * v2x) + v0z * (v1x * v2y - v1y * v2x);
		if (std::abs(det) <= eps)
			return Orient::On;

		if (det > 0)
			return Orient::In;

		return Orient::Out;
	}
}
