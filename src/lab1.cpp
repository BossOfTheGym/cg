#include "lab1.h"
#include "lab1-gui.h"

#include "quadtree.h"
#include "primitive.h"
#include "graphics-res/graphics-res-util.h"

#include <vector>
#include <cassert>
#include <iostream>

namespace
{
	constexpr const u32 max_points = 1'000'000;

	class Lab1Impl
	{
	public:
		Lab1Impl(Lab1* lab) : m_lab(lab)
		{}

		~Lab1Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab1Gui(max_points, 0u));
			assert(m_gui != nullptr);
			m_generatePointsConn = m_gui->generatePoints    .connect([&](u32 pts){ generatePoints(pts); });
			m_goBackConn         = m_gui->returnBack        .connect([&](){ returnBack(); });
			m_frameChangedConn   = m_gui->frameParamsChanged.connect([&](f32 x0, f32 x1, f32 y0, f32 y1) { frameParamsChanged(x0, x1, y0, y1); });

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			m_generatePointsConn.release();
			m_goBackConn.release();
			m_frameChangedConn.release();
			m_gui.reset();

			m_initialized = false;
		}

		AppAction execute()
		{
			m_gui->draw();
			if (m_goBack)
			{
				m_goBack = false;
				return AppAction{ActionType::Pop};
			}
			return {}; // Nothing
		}

	private: // callbacks
		void generatePoints(u32 amount)
		{
			// TODO
			std::cout << "generate points: amout" << std::endl;
		}

		void returnBack()
		{
			m_goBack = true;
		}

		void frameParamsChanged(f32 x0, f32 x1, f32 y0, f32 y1)
		{
			// TODO
			std::cout << "frame Changed: " << x0 << " " << x1 << " " << y0 << " " << y1 << std::endl;
		}


	private:
		Lab1* m_lab{};

		std::unique_ptr<Lab1Gui> m_gui;
		sig::Connection m_goBackConn;
		sig::Connection m_frameChangedConn;
		sig::Connection m_generatePointsConn;
		bool m_goBack{false};		

		prim::vec4 m_frameParams{};

		bool m_initialized{false};

		// TODO
		// quadtree
		// quadtree query
		// 
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
{
	m_impl.reset(new Lab1Impl(this));
	assert(m_impl != nullptr);
}

Lab1::~Lab1()
{
	deinit();

	m_impl.reset();
}

bool Lab1::init()
{
	return m_impl->init();
}

void Lab1::deinit()
{
	m_impl->deinit();
}

AppAction Lab1::execute()
{
	return m_impl->execute();
}
