#include "quadtree_test.h"

#include "quadtree.h"

void test_simple_quadtree()
{
	using vec2 = qtree::vec2;

	i32 div = 4;

	vec2 v0{-1.0, -1.0};
	vec2 v1{+1.0, +1.0};

	qtree::QuadTree<> qtr({{-1.0,-1.0}, {+1.0, +1.0}});

	auto vecs = create_vecs(v0, v1, div);
	for (auto& vec : vecs)
	{
		qtr.insert(&vec);
	}
	for (auto& vec : vecs)
	{
		qtr.remove(&vec);
	}
}