#pragma once

#include "primitive.h"

#include <vector>
#include <limits>
#include <algorithm>


namespace bezier
{
	using Float = prim::Float;
	using Vec2  = prim::Vec2;
	using Vec3  = prim::Vec3;

	template<class vec>
	struct Patch2D
	{
		vec v0{};
		vec v1{};
		vec v2{};
		vec v3{};
	};

	template<class vec>
	vec eval2d(const vec& v0, const vec& v1, const vec& v2, const vec& v3, Float t)
	{
		using value = typename vec::value_type;

		value m = 1.0 - t;
		value mmm  = m * m * m;
		value tmm3 = t * m * m * 3.0f;
		value ttm3 = t * t * m * 3.0f;
		value ttt  = t * t * t;
		return v0 * mmm + v1 * tmm3 + v2 * ttm3 + v3 * ttt;
	}

	template<class vec>
	vec eval2d(const vec (&v)[4], Float t)
	{
		return eval2d(v[0], v[1], v[2], v[3], t);
	}

	template<class vec>
	vec eval2d(const Patch2D<vec>& patch, Float t)
	{
		return eval2d(patch.v0, patch.v1, patch.v2, patch.v3, t);
	}
	
	template<class vec>
	vec c1connect2d_prev(const vec& v, const vec& next)
	{
		auto dv = next - v;

		return v - dv;
	}

	template<class vec>
	vec c1connect2d_next(const vec& prev, const vec& v)
	{
		auto dv = v - prev;

		return v + dv;
	}


	struct Patch3D
	{
		Vec3 v[16]{};
	};

	// v ^
	//   | v12 v13 v14 v15
	//   | v8  v9  v10 v11
	//   | v4  v5  v6  v7 
	//   | v0  v1  v2  v3
	//   '----------------->
	//                     u
	// t = (u, v)
	template<class vec_t>
	vec_t eval3d(
		const vec_t& v0 , const vec_t& v1 , const vec_t& v2 , const vec_t& v3 ,
		const vec_t& v4 , const vec_t& v5 , const vec_t& v6 , const vec_t& v7 ,
		const vec_t& v8 , const vec_t& v9 , const vec_t& v10, const vec_t& v11,
		const vec_t& v12, const vec_t& v13, const vec_t& v14, const vec_t& v15, Float u, Float v)
	{
		auto vu0 = eval2d(v0 , v1 , v2 , v3 , u);
		auto vu1 = eval2d(v4 , v5 , v6 , v7 , u);
		auto vu2 = eval2d(v8 , v9 , v10, v11, u);
		auto vu3 = eval2d(v12, v13, v14, v15, u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}

	template<class vec_t>
	vec_t eval3d(const vec_t (&vs)[16], Float u, Float v)
	{
		auto vu0 = eval2d(vs[0] , vs[1] , vs[2] , vs[3] , u);
		auto vu1 = eval2d(vs[4] , vs[5] , vs[6] , vs[7] , u);
		auto vu2 = eval2d(vs[8] , vs[9] , vs[10], vs[11], u);
		auto vu3 = eval2d(vs[12], vs[13], vs[14], vs[15], u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}
}
