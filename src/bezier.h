#pragma once

#include "primitive.h"

#include <vector>
#include <limits>
#include <algorithm>


namespace bezier
{
	using Float = prim::Float;
	using vec2  = prim::vec2;
	using vec3  = prim::vec3;


	template<class vec>
	vec eval2d(const vec& v0, const vec& v1, const vec& v2, const vec& v3, Float t)
	{
		using value = typename vec::value_type;

		value m = 1.0 - t;

		return v0 * m * m * m
			+ v1 * t * m * m * 3.0f
			+ v2 * t * t * m * 3.0f
			+ v3 * t * t * t;
	}

	template<class vec>
	vec eval2d(const vec (&v)[4], Float t)
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


	struct Patch2D
	{
		vec2 v0{};
		vec2 v1{};
		vec2 v2{};
		vec2 v3{};
	};

	class Curve2D
	{
	public:
		static constexpr u32 MISS = std::numeric_limits<u32>::max();

		class Iterator
		{
			friend class Curve2D;

			Iterator(Curve2D* curve, u32 index) noexcept : m_curve{curve}, m_index{index}
			{}

		public:
			using iterator_category = std::input_iterator_tag;
			using value_type      = Patch2D;
			using difference_type = u32;
			using pointer         = Patch2D;
			using reference       = Patch2D;


		public:
			Iterator() noexcept
			{}

			Patch2D operator * ()
			{
				auto& verts = m_curve->m_vertices;

				return {verts[m_index], verts[m_index + 1], verts[m_index + 2], verts[m_index + 3]};
			}

			Patch2D operator -> ()
			{
				return **this;
			}


			Iterator& operator ++ ()
			{
				if (m_index < m_curve->m_vertices.size())
				{
					m_index += 3;
				}
				return *this;
			}

			Iterator operator ++ (int)
			{
				auto it{*this};

				++(*this);

				return it;
			}

			Iterator& operator -- ()
			{
				if (m_index > 0)
				{
					m_index -= 3;
				}
				return *this;
			}

			Iterator operator -- (int)
			{
				auto it{*this};

				--*this;

				return it;
			}


			Iterator& operator -= (difference_type diff)
			{
				m_index -= std::min(m_index, diff * 3);

				return *this;
			}

			Iterator& operator += (difference_type diff)
			{
				m_index += std::min(m_index, diff * 3);

				return *this;
			}

			Iterator operator - (difference_type diff)
			{
				auto it{*this};

				return it -= diff;
			}

			Iterator operator - (difference_type diff)
			{
				auto it{*this};

				return it += diff;
			}


			difference_type operator - (Iterator it)
			{
				return m_index - it.m_index;
			}


			bool operator == (Iterator it)
			{
				return m_index == it.m_index;
			}

			bool operator != (Iterator it)
			{
				return m_index != it.m_index;
			}

			bool operator < (Iterator it)
			{
				return m_index < it.m_index;
			}

			bool operator > (Iterator it)
			{
				return m_index > it.m_index;
			}


		private:
			Curve2D* m_curve{nullptr};
			u32 m_index{MISS};
		};
			

	private:
		enum class Type : u32
		{
			V0 = 0,
			V1 = 1,
			V2 = 2,
			V3 = 3,
		};

		static Type point_type(u32 i)
		{
			if (i == 0)
			{
				return Type::V0;
			}
			return Type{(i - 1) % 3 + 1};
		}

		static u32 segment(u32 i)
		{
			if (i == 0)
			{
				return 0u;
			}
			return (i - 1) / 3;
		}


	public:
		u32 capture(const vec2& hit, Float dist)
		{
			for (u32 i = 0; i < m_vertices.size(); i++)
			{
				auto dv = prim::abs(hit - m_vertices[i]);
				if (dv.x <= dist && dv.y <= dist)
				{
					return i;
				}
			}
			return MISS;
		}

		bool captures(u32 index, const vec2& hit, Float dist)
		{
			if (index >= m_vertices.size())
			{
				return;
			}

			auto dv = prim::abs(hit - m_vertices[index]);
			return dv.x <= dist && dv.y <= dist;
		}

		void addPatch(const vec2& v0, const vec2& v1)
		{
			auto dv = (v1 - v0) / 3.0f;

			m_vertices.push_back(v0);
			m_vertices.push_back(v0 + dv);
			m_vertices.push_back(v0 + 2.0f * dv);
			m_vertices.push_back(v1);
		}

		void change(u32 index, const vec2& value)
		{
			if (index >= m_vertices.size())
			{
				return;
			}

			m_vertices[index] = value;

			auto seg  = segment(index);
			auto segs = segment(m_vertices.size());
			switch(point_type(index))
			{
				case Type::V1:
				{
					if (seg != 0)
					{
						m_vertices[index - 2] = c1connect2d_prev(m_vertices[index - 1], m_vertices[index]);
					}					
					break;
				}

				[[fallthrough]]
				case Type::V2:
				case Type::V3:
				{
					if (seg != segs - 1)
					{
						m_vertices[index + 2] = c1connect2d_next(m_vertices[index], m_vertices[index + 1]);	
					}
					break;
				}
			}
		}


		Iterator begin()
		{
			return Iterator{this, 0};
		}

		Iterator end()
		{
			return Iterator{this, m_vertices.size() != 0 ? m_vertices.size() - 1 : 0};
		}


	private:
		std::vector<vec2> m_vertices;
	};

	// v ^
	//   | v12 v13 v14 v15
	//   | v8  v9  v10 v11
	//   | v4  v5  v6  v7 
	//   | v0  v1  v2  v3
	//   '----------------->
	//                     u
	// t = (u, v)
	vec3 eval3d(
		const vec3& v0 , const vec3& v1 , const vec3& v2 , const vec3& v3 ,
		const vec3& v4 , const vec3& v5 , const vec3& v6 , const vec3& v7 ,
		const vec3& v8 , const vec3& v9 , const vec3& v10, const vec3& v11,
		const vec3& v12, const vec3& v13, const vec3& v14, const vec3& v15, Float u, Float v)
	{
		auto vu0 = eval2d(v0 , v1 , v2 , v3 , u);
		auto vu1 = eval2d(v4 , v5 , v6 , v7 , u);
		auto vu2 = eval2d(v8 , v9 , v10, v11, u);
		auto vu3 = eval2d(v12, v13, v14, v15, u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}

	vec3 eval3d(const vec3 (&vs)[16], Float u, Float v)
	{
		auto vu0 = eval2d(vs[0] , vs[1] , vs[2] , vs[3] , u);
		auto vu1 = eval2d(vs[4] , vs[5] , vs[6] , vs[7] , u);
		auto vu2 = eval2d(vs[8] , vs[9] , vs[10], vs[11], u);
		auto vu3 = eval2d(vs[12], vs[13], vs[14], vs[15], u);

		return eval2d(vu0, vu1, vu2, vu3, v);
	}

	struct Patch3D
	{
		vec3 v[16]{};
	};

	class CurvePatch3D
	{
	public:
		static constexpr u32 MISS = std::numeric_limits<u32>::max();

	public:
		CurvePatch3D(const Patch3D& patch) : m_patch{patch}
		{}

	public:
		u32 capture(const vec3& hit, Float dist)
		{
			for (u32 i = 0; i < 16; i++)
			{
				auto dv = prim::abs(hit - m_patch.v[i]);
				if (dv.x <= dist && dv.y <= dist && dv.z <= dist)
				{
					return i;
				}
			}
			return MISS;
		}

		bool captures(u32 index, const vec3& hit, Float dist)
		{
			if (index >= 16)
			{
				return false;
			}

			auto dv = prim::abs(hit - m_patch.v[index]);
			return dv.x <= dist && dv.y <= dist && dv.z <= dist;
		}

		void change(u32 index, const vec3& value)
		{
			if (index >= 16)
			{
				return;
			}

			m_patch.v[index] = value;
		}

	private:
		Patch3D m_patch;
	};
}
