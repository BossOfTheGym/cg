#include "choose-lab.h"
#include "choose-gui.h"

#include "app.h"
#include "primitive.h"
#include "main-window.h"
#include "prog-fractal.h"
#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res.h"
#include "graphics-res/graphics-res-util.h"

using namespace prim;

namespace
{
	class ChooseGuiImpl
	{
	public:
		ChooseGuiImpl(ChooseLab* chooseLab) : m_host(chooseLab), m_window(chooseLab->app().window())
		{}

		~ChooseGuiImpl()
		{
			deinit();
		}


	public:
		bool init()
		{
			if (m_initialized)
				return true;

			initGui();
			initGfx();
			initState();

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			deinitState();
			deinitGfx();
			deinitGui();

			m_initialized = false;
		}

		AppAction execute()
		{
			doGui();
			render();
			return handleState();
		}

		void pause()
		{
			resetState();
		}

		void resume()
		{
			resetState();
		}

	private: // init & deinit
		void initGui()
		{
			m_gui.reset(new ChooseGui());
			assert(m_gui != nullptr);
			m_labChangedConn = m_gui->optionChosen.connect([&](std::string lab){ onLabChosen(std::move(lab)); });
			m_exitAppConn    = m_gui->exitApp     .connect([&](){ onExitApp(); });
		}

		void deinitGui()
		{
			m_labChangedConn.release();
			m_exitAppConn.release();
			m_gui.reset();
		}

		void initGfx()
		{
			auto stat = res::try_create_vertex_array(m_dummyArray);
			assert(stat);

			m_progFractal.reset(new ProgFractal());
			assert(m_progFractal != nullptr);
		}

		void deinitGfx()
		{
			m_progFractal.reset();
			m_dummyArray.reset();
		}

		void initState()
		{
			resetState();
		}

		void deinitState()
		{
			resetState();
		}

	private: // act
		AppAction handleState()
		{
			if (m_needExit)
			{
				return AppAction{ActionType::Exit};
			}
			if (m_labChosen)
			{
				m_labChosen = false;
				return AppAction{ActionType::Push, std::move(m_chosenLab)};
			}
			return AppAction{};
		}


	private: // gui & callbacks
		void doGui()
		{
			m_gui->draw();
		}

		void onLabChosen(std::string lab)
		{
			m_labChosen = true;
			m_chosenLab = std::move(lab);
		}

		void onExitApp()
		{
			m_needExit = true;
		}


	private: // render
		void render()
		{
			auto [w, h] = m_window->framebufferSize();
			auto t = glfw::get_time();

			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			m_progFractal->use();
			m_progFractal->setResolution({w, h});
			m_progFractal->setTime(t - m_t0);

			glBindVertexArray(m_dummyArray.id);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		
	private: // utility
		void resetState()
		{
			m_t0 = glfw::get_time();
			m_labChosen = false;
			m_chosenLab.clear();
		}


	private:
		// app state
		ChooseLab* m_host{};
		MainWindow* m_window{};
		bool m_initialized{false};

		// gui
		std::unique_ptr<ChooseGui> m_gui;
		sig::Connection m_labChangedConn;
		sig::Connection m_exitAppConn;

		// gfx
		res::VertexArray m_dummyArray;
		std::unique_ptr<ProgFractal> m_progFractal;

		// state
		std::string m_chosenLab;
		f32 m_t0{};
		bool m_needExit{false};
		bool m_labChosen{false};
	};
}

ChooseLab::ChooseLab(App* owner) : AppState(owner)
{
	m_impl.reset(new ChooseGuiImpl(this));
	assert(m_impl != nullptr);
}

ChooseLab::~ChooseLab()
{
	deinit();
}


// AppState
bool ChooseLab::init()
{
	return m_impl->init();
}

void ChooseLab::deinit()
{
	m_impl->deinit();
}

AppAction ChooseLab::execute()
{
	return m_impl->execute();
}

void ChooseLab::pause()
{
	m_impl->pause();
}

void ChooseLab::resume()
{
	m_impl->resume();
}
