#include "lab4.h"
#include "lab4-gui.h"

#include <cassert>
#include <iostream>

namespace
{
	constexpr const u32 max_points = 1'00'000;

	class Lab4Impl
	{
	public:
		Lab4Impl(Lab4* lab) : m_lab(lab)
		{}

		~Lab4Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab4Gui(max_points, 0u));
			assert(m_gui != nullptr);
			m_generateConn = m_gui->generate.connect([&](u32 count){ generated(count);});
			m_buildConn    = m_gui->build   .connect([&](){ built(); });
			m_clearConn    = m_gui->clear   .connect([&](){ cleared(); });
			m_backConn     = m_gui->back    .connect([&](){ back(); });

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			m_generateConn.release();
			m_buildConn.release();
			m_clearConn.release();
			m_backConn.release();
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
			return {};
		}

	private:
		void generated(u32 count)
		{
			// TODO
			std::cout << "generated: " << count << std::endl;
		}

		void built()
		{
			// TODO
			std::cout << "built" << std::endl;
		}

		void cleared()
		{
			// TODO
			std::cout << "cleared" << std::endl;
		}

		void back()
		{
			m_goBack = true;
		}

	private:
		Lab4* m_lab{};

		std::unique_ptr<Lab4Gui> m_gui;
		sig::Connection m_generateConn;
		sig::Connection m_buildConn;
		sig::Connection m_clearConn;
		sig::Connection m_backConn;
		bool m_goBack{false};

		bool m_initialized{false};
	};
}


Lab4::Lab4(App* app) : AppState(app)
{
	m_impl.reset(new Lab4Impl(this));
	assert(m_impl != nullptr);
}

Lab4::~Lab4()
{
	m_impl.reset();
}

bool Lab4::init()
{
	return m_impl->init();
}

void Lab4::deinit()
{
	m_impl->deinit();
}

AppAction Lab4::execute()
{
	return m_impl->execute();
}