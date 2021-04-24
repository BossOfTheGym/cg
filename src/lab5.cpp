#include "lab5.h"
#include "lab5-gui.h"

#include <cassert>
#include <iostream>

namespace
{
	class Lab5Impl
	{
	public:
		Lab5Impl(Lab5* lab) : m_lab(lab)
		{}

		~Lab5Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab5Gui());
			assert(m_gui != nullptr);
			m_backConn     = m_gui->back    .connect([&](){ back(); });

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

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
		void back()
		{
			m_goBack = true;
		}

	private:
		Lab5* m_lab{};

		std::unique_ptr<Lab5Gui> m_gui;
		sig::Connection m_backConn;
		bool m_goBack{false};

		bool m_initialized{false};
	};
}


Lab5::Lab5(App* app) : AppState(app)
{
	m_impl.reset(new Lab5Impl(this));
	assert(m_impl != nullptr);
}

Lab5::~Lab5()
{
	m_impl.reset();
}

bool Lab5::init()
{
	return m_impl->init();
}

void Lab5::deinit()
{
	m_impl->deinit();
}

AppAction Lab5::execute()
{
	return m_impl->execute();
}
