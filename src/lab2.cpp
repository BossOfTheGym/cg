#include "lab2.h"
#include "lab2-gui.h"

#include <memory>
#include <cassert>
#include <iostream>

namespace
{
	constexpr const u32 max_segments = 1'00'000;

	class Lab2Impl
	{
	public:
		Lab2Impl(Lab2* lab) : m_lab(lab)
		{}

		~Lab2Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab2Gui(max_segments, 0u));
			assert(m_gui != nullptr);
			m_generateConn  = m_gui->generate .connect([&](u32 count){ generate(count); });
			m_intersectConn = m_gui->intersect.connect([&](){ intersect(); });;
			m_clearConn     = m_gui->clear    .connect([&](){ clear(); });
			m_backConn      = m_gui->back     .connect([&](){ back(); });

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			m_generateConn.release();
			m_intersectConn.release();
			m_clearConn.release();
			m_backConn.release();

			m_gui.reset();
		}

		AppAction execute()
		{
			m_gui->draw();
			if (m_goBack)
			{
				m_goBack = false;
				return AppAction{ActionType::Pop};
			}
			return {};
		}

	private:
		void generate(u32 count)
		{
			// TODO
			std::cout << "generate:" << count << std::endl;
		}

		void intersect()
		{
			// TODO
			std::cout << "intersect" << std::endl;
		}

		void clear()
		{
			// TODO
			std::cout << "clear" << std::endl;
		}

		void back()
		{
			m_goBack = true;
		}

	private:
		Lab2* m_lab{};

		std::unique_ptr<Lab2Gui> m_gui;
		sig::Connection m_generateConn;
		sig::Connection m_intersectConn;
		sig::Connection m_clearConn;
		sig::Connection m_backConn;

		bool m_goBack{false};

		bool m_initialized{false};
	};
}


Lab2::Lab2(App* app) : AppState(app)
{
	m_impl.reset(new Lab2Impl(this));
	assert(m_impl != nullptr);
}

Lab2::~Lab2()
{
	deinit();

	m_impl.reset();
}

bool Lab2::init()
{
	return m_impl->init();
}

void Lab2::deinit()
{
	m_impl->deinit();
}

AppAction Lab2::execute()
{
	return m_impl->execute();
}
