#include "choose-gui.h"

#include "app.h"
#include "app-state.h"
#include "state-register.h"

#include "primitive.h"
#include "main-window.h"
#include "prog-fractal.h"
#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res.h"
#include "graphics-res/graphics-res-util.h"

using namespace prim;

class ChooseLab : public AppState
{
public:
	ChooseLab(App* app) : AppState(app), m_window(app->window())
	{}

	~ChooseLab()
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

REGISTER_STATE(choose-lab, ChooseLab)
