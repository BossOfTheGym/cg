#include "lab3.h"
#include "lab3-gui.h"

#include <memory>
#include <cassert>
#include <iostream>

namespace
{
	constexpr const u32 max_points = 1'000'000;

	class Lab3Impl
	{
	public:
		Lab3Impl(Lab3* lab) : m_lab(lab)
		{}

		~Lab3Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab3Gui(max_points, 0u));
			assert(m_gui != nullptr);
			m_generateConn = m_gui->generate.connect([&](u32 count){ generate(count); });
			m_buildConn    = m_gui->build   .connect([&](){ build(); });;
			m_clearConn    = m_gui->clear   .connect([&](){ clear(); });
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

		void build()
		{
			// TODO
			std::cout << "build" << std::endl;
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
		Lab3* m_lab{};

		std::unique_ptr<Lab3Gui> m_gui;
		sig::Connection m_generateConn;
		sig::Connection m_buildConn;
		sig::Connection m_clearConn;
		sig::Connection m_backConn;

		bool m_goBack{false};

		bool m_initialized{false};
	};
}


Lab3::Lab3(App* app) : AppState(app)
{
	m_impl.reset(new Lab3Impl(this));
	assert(m_impl != nullptr);
}

Lab3::~Lab3()
{
	deinit();

	m_impl.reset();
}

bool Lab3::init()
{
	return m_impl->init();
}

void Lab3::deinit()
{
	m_impl->deinit();
}

AppAction Lab3::execute()
{
	return m_impl->execute();
}
