#pragma once

#include <vector>
#include <limits>
#include <algorithm>

namespace bezier
{
	template<class vec, class val>
	vec eval2d(const vec& v0, const vec& v1, const vec& v2, const vec& v3, val t)
	{
		using value = typename vec::value_type;

		value t0 = t;
		value m0 = 1.0 - t;

		vec tmp0 = v0 * m0 + v1 * t0;
		vec tmp1 = v1 * m0 + v2 * t0;
		vec tmp2 = v2 * m0 + v3 * t0;
		tmp0 = tmp0 * m0 + tmp1 * t0;
		tmp1 = tmp1 * m0 + tmp2 * t0;

		return tmp0 * m0 + tmp1 * t0;
	}

	template<class vec, class val>
	vec eval2d(const vec (&v)[4], val t)
	{
		return eval2d(v[0], v[1], v[2], v[3], t);
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


	template<class vec>
	struct Patch2D
	{
		using value = typename vec::value_type;

		vec eval(value t)
		{
			return eval2d(vs, t);
		}

		vec vs[4]{};
	};


	// v ^
	//   | v12 v13 v14 v15
	//   | v8  v9  v10 v11
	//   | v4  v5  v6  v7 
	//   | v0  v1  v2  v3
	//   '----------------->
	//                     u
	// t = (u, v)
	template<class vec, class val>
	vec eval3d(
		const vec& v0 , const vec& v1 , const vec& v2 , const vec& v3 ,
		const vec& v4 , const vec& v5 , const vec& v6 , const vec& v7 ,
		const vec& v8 , const vec& v9 , const vec& v10, const vec& v11,
		const vec& v12, const vec& v13, const vec& v14, const vec& v15, val u, val v)
	{
		auto vu0 = eval2d(v0 , v1 , v2 , v3 , u);
		auto vu1 = eval2d(v4 , v5 , v6 , v7 , u);
		auto vu2 = eval2d(v8 , v9 , v10, v11, u);
		auto vu3 = eval2d(v12, v13, v14, v15, u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}

	template<class vec, class val>
	vec eval3d(const vec (&vs)[16], val u, val v)
	{
		auto vu0 = eval2d(vs[0] , vs[1] , vs[2] , vs[3] , u);
		auto vu1 = eval2d(vs[4] , vs[5] , vs[6] , vs[7] , u);
		auto vu2 = eval2d(vs[8] , vs[9] , vs[10], vs[11], u);
		auto vu3 = eval2d(vs[12], vs[13], vs[14], vs[15], u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}


	template<class vec>
	struct Patch3D
	{
		using value = typename vec::value_type;

		vec eval(value u, value v)
		{
			return eval3d(vs, u, v);
		}

		vec vs[16]{};
	};
}
