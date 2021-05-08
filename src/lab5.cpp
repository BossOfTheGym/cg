#include "lab5-gui.h"

#include "app.h"
#include "app-state.h"
#include "state-register.h"

#include "primitive.h"
#include "main-window.h"
#include "prog-unicolor-prim3d.h"
#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cmath>
#include <memory>
#include <random>
#include <numeric>
#include <cassert>
#include <utility>
#include <iostream>

using namespace prim;

class Lab5 : public AppState
{
public:
	Lab5(App* app) : AppState(app), m_window(app->window())
	{}

	~Lab5()
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
		auto act = handleState();
		render();
		return act;
	}

private: // init & deinit
	void initGui()
	{
		m_gui.reset(new Lab5Gui());
		assert(m_gui != nullptr);
		m_backConn = m_gui->back.connect([&](){ onBack(); });
	}

	void deinitGui()
	{
		m_backConn.release();
		m_gui.reset();
	}

	void initGfx()
	{
		bool stat{};

		// cube
		vec3 vertices[] = 
		{
			{-1.0, -1.0, -1.0},
			{-1.0, -1.0, +1.0},
			{-1.0, +1.0, -1.0},
			{-1.0, +1.0, +1.0},
			{+1.0, -1.0, -1.0},
			{+1.0, -1.0, +1.0},
			{+1.0, +1.0, -1.0},
			{+1.0, +1.0, +1.0},
		};

		u8 indices[] = 
		{
			0, 1, 1, 3, 3, 2, 2, 0,
			4, 5, 5, 7, 7, 6, 6, 4,
			1, 5, 3, 7, 2, 6, 0, 4,
		};

		m_elements  = sizeof(indices) / sizeof(indices[0]);
		m_indOffset = sizeof(vertices);

		stat = res::try_create_storage_buffer(m_cubeBuffer, sizeof(vertices) + sizeof(indices), GL_MAP_WRITE_BIT);
		assert(stat);

		auto* vertPtr = glMapNamedBufferRange(m_cubeBuffer.id, 0, sizeof(vertices), GL_MAP_WRITE_BIT);
		std::memcpy(vertPtr, vertices, sizeof(vertices));
		glUnmapNamedBuffer(m_cubeBuffer.id);

		auto* indPtr = glMapNamedBufferRange(m_cubeBuffer.id, sizeof(vertices), sizeof(indices), GL_MAP_WRITE_BIT);
		std::memcpy(indPtr, indices, sizeof(indices));
		glUnmapNamedBuffer(m_cubeBuffer.id);

		// sync buffer memory operations
		res::FenceSync sync;
		stat = res::try_create_sync(sync);
		assert(stat);

		// array
		stat = res::try_create_vertex_array(m_cubeArray);
		assert(stat);
		glBindVertexArray(m_cubeArray.id);
		glEnableVertexAttribArray(0);
		glBindVertexBuffer(0, m_cubeBuffer.id, 0, sizeof(vec3));
		glVertexBindingDivisor(0, 0);
		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeBuffer.id);

		// prog
		m_progUnicolor3D.reset(new ProgUnicolorPrim3D());
		assert(m_progUnicolor3D != nullptr);

		// wait sync
		GLenum result{};
		while(true)
		{
			result = glClientWaitSync(sync.id, GL_SYNC_FLUSH_COMMANDS_BIT, 50);
			if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
				break;
		}
		sync.del();
	}

	void deinitGfx()
	{
		m_progUnicolor3D.reset();
		m_cubeBuffer.reset();
		m_cubeArray.reset();
	}

	void initState()
	{
		m_t  = glfw::get_time();
		m_dt = 0.0;
		m_needGoBack = false;
	}

	void deinitState()
	{
		m_needGoBack = false;	
	}


private: // act
	AppAction handleState()
	{
		auto t = glfw::get_time();
		m_dt = t - m_t;
		m_t = t;

		if (m_needGoBack)
		{
			m_needGoBack = false;
			return AppAction{ActionType::Pop};
		}
		return {};
	}


private: // gui
	void doGui()
	{
		m_gui->draw();
	}

	void onBack()
	{
		m_needGoBack = true;
	}


private: // render
	void render()
	{
		auto [w, h] = m_window->framebufferSize();

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glViewport(0, 0, w, h);

		m_progUnicolor3D->use();
		m_progUnicolor3D->setPrimColor(m_color);
		m_progUnicolor3D->setProj(transform());

		glBindVertexArray(m_cubeArray.id);
		glDrawElements(GL_LINES, m_elements, GL_UNSIGNED_BYTE, (void*)m_indOffset);
	}

private: // utility
	mat4 transform()
	{
		auto [w, h] = m_window->framebufferSize();
		f32 a = (f32)w / h;
		f32 m = 2.0f;
		mat4 proj = ortho(-m * a, +m * a, -m, +m, 0.1f, 100.0f);
		mat4 cube = rotate(mat4(1.0), m_t / 3.0f, vec3(0.0, 1.0, 0.0));
		mat4 view = lookAt(vec3(3.0), vec3(0.0), vec3(0.0, 1.0, 0.0));
		return proj * view * cube;
	}

private:
	// app state
	Lab5*       m_lab{};
	MainWindow* m_window{};
	bool m_initialized{false};

	// gui state
	std::unique_ptr<Lab5Gui> m_gui;
	sig::Connection m_backConn;

	// gfx
	u32 m_indOffset{};
	u32 m_elements{};
	res::Buffer m_cubeBuffer;
	res::VertexArray m_cubeArray; // primitive: GL_LINES, indices: GL_UNSIGNED_BYTE
	std::unique_ptr<ProgUnicolorPrim3D> m_progUnicolor3D;
	vec4 m_color{0.0, 0.0, 0.0, 1.0};

	// state
	f32 m_t{};
	f32 m_dt{};
	bool m_needGoBack{};
};

REGISTER_STATE(lab5, Lab5)
