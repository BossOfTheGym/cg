#include "quadtree_test.h"
#include "test_util.h"

#include "quadtree.h"

#include <random>


namespace
{
	template<class T>
	class Allocator
	{
	public:
	    template<class ... Args>
	    T* alloc(Args&& ... args)
	    {
	        if constexpr(std::is_aggregate_v<T>)
	            return ::new T{std::forward<Args>(args)...};
	        else
	            return ::new T(std::forward<Args>(args)...);
	    }
	
	    void dealloc(T* ptr)
	    {
	        ::delete ptr;
	    }
	};

	template<class vec>
	struct Position
	{
		vec operator() (const vec& vec)
		{
			return vec;
		}

		vec operator() (const vec& vec) const
		{
			return vec;
		}
	};
}

void test_simple_quadtree()
{
	using Vec2 = qtree::Vec2;

	i32 div = 512;

	Vec2 v0{-1.0, -1.0};
	Vec2 v1{+1.0, +1.0};

	using Helper = qtree::Helper<Vec2, Position<Vec2>, Allocator>;

	Helper::Tree qtr({{-1.0,-1.0}, {+1.0, +1.0}});


	auto vecs = create_vecs(v0, v1, div);
	for (auto& vec : vecs)
	{
		qtr.insert(vec);
	}
	for (auto& vec : vecs)
	{
		qtr.remove(vec);
	}
}