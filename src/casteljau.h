#pragma once

#include "primitive.h"

#include <vector>
#include <limits>
#include <algorithm>

namespace casteljau
{
	template<class vec_t>
	class Patch
	{
	public:
		using vec   = vec_t;
		using value = typename vec::value_type;

	public:
		vec eval(value t)
		{
			int n = pts.size();

			m_b = pts;
			for (int j = 0; j < n - 1; j++)
			{
				for (int i = 0; i < n - j - 1; i++)
					m_b[i] = m_b[i] * (1 - t) + m_b[i + 1] * t;
			}
			return m_b[0];
		}

	public:
		std::vector<vec> pts;

	private:
		std::vector<vec> m_b;
	};
}
