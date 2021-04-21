#include "lab1.h"
#include "lab1-gui.h"

#include "quadtree.h"
#include "primitive.h"
#include "graphics-res/graphics-res-util.h"

#include <vector>
#include <cassert>

namespace
{
	using Point = prim::Vec2;

	class Points
	{};

	class Lab1Impl
	{
	public:
		Lab1Impl(Lab1* lab)
		{}

	public:
		AppAction execute()
		{
			return {};
		}

	private:
		// quadtree
		// quadtree query
		// points
		// gfx-points : class that will set point colors
		// * vertex array : pos, color
		// 
		// Framebuffer
		// * attached texture
		// rect prog
		// point prog
	};
}

Lab1::Lab1(App* app) : AppState(app)
{}

Lab1::~Lab1()
{
	deinit();
}

bool Lab1::init()
{
	m_impl.reset(new Lab1Impl(this));
	assert(m_impl != nullptr);

	return true;
}

void Lab1::deinit()
{
	if (m_impl != nullptr)
		m_impl.reset();
}

AppAction Lab1::execute()
{
	assert(m_impl != nullptr);

	return m_impl->execute();
}
