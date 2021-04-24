#include "lab6.h"
#include "lab6-gui.h"

#include <cassert>
#include <iostream>

namespace
{
	class Lab6Impl
	{
	public:
		Lab6Impl(Lab6* lab) : m_lab(lab)
		{}

		~Lab6Impl()
		{
			deinit();
		}

	public:
		bool init()
		{
			if (m_initialized)
				return true;

			m_gui.reset(new Lab6Gui());
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
		Lab6* m_lab{};

		std::unique_ptr<Lab6Gui> m_gui;
		sig::Connection m_backConn;
		bool m_goBack{false};

		bool m_initialized{false};
	};
}


Lab6::Lab6(App* app) : AppState(app)
{
	m_impl.reset(new Lab6Impl(this));
	assert(m_impl != nullptr);
}

Lab6::~Lab6()
{
	m_impl.reset();
}

bool Lab6::init()
{
	return m_impl->init();
}

void Lab6::deinit()
{
	m_impl->deinit();
}

AppAction Lab6::execute()
{
	return m_impl->execute();
}
