#include "app.h"
#include "app-state.h"
#include "state-register.h"

#include "lab7-gui.h"
#include "main-window.h"

#include "bezier.h"
#include "primitive.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res.h"
#include "graphics-res/graphics-res-util.h"

#include "gfx-res.h"

#include "prog-unicolor-prim3d.h"


using namespace prim;

using GfxSBuffer3   = GfxSBuffer<vec3>;
using GfxSBufferU16 = GfxSBuffer<u16>;

namespace
{
	constexpr u32 patch_split = 48u; // multiple of 3
	constexpr u32 invalid = std::numeric_limits<u32>::max();
}

class Lab7 : public AppState
{
public:
	Lab7(App* app) : AppState(app), m_window(app->window())
	{}

	~Lab7()
	{
		deinit();
	}


public: // AppState
	virtual bool init() override
	{
		if (m_initialized)
			return true;

		initGui();
		initPatch();
		initGfx();
		initState();

		m_initialized = true;

		return true;
	}

	virtual void deinit() override
	{
		if (!m_initialized)
			return;

		deinitState();
		deinitGfx();
		deinitPatch();
		deinitGui();

		m_initialized = false;
	}

	virtual AppAction execute() override
	{
		doGui();
		auto action = handleState();
		render();
		return action;
	}

private: // init & deinit
	void initGui()
	{
		m_gui.reset(new Lab7Gui(0.05, -1000.0, +1000.0, &m_patch));
		assert(m_gui != nullptr);
		m_vertexChangedConn = m_gui->vertexChanged.connect([&](u32 id){ onVertexChanged(id); });
		m_returnBackConn    = m_gui->returnBack.connect([&](){ onReturnBack(); });
	}

	void deinitGui()
	{
		m_vertexChangedConn.release();
		m_returnBackConn.release();
		m_gui.reset();
	}

	void initPatch()
	{
		for (int i = 0; i <= 3; i++)
		{
			for (int j = 0; j <= 3; j++)
			{
				m_patch.vs[i * 4 + j] = vec3(-1.0 + 2.0 * i / 3, 0.0, -1.0 + 2.0 * j / 3);
			}
		}
	}

	void deinitPatch()
	{}

	void initGfx()
	{
		// buffers
		m_lineSplit = patch_split;
		m_totalVertices = 2 * 4 * (m_lineSplit + 1);
		m_totalElements = 2 * 4 * (m_lineSplit + 1 + 1);
		m_indicesOffset = 0;
		m_restartValue = (u16)-1;

		m_vertexBuffer.reset(new GfxSBuffer3(m_totalVertices));
		assert(m_vertexBuffer != nullptr);

		m_indicesBuffer.reset(new GfxSBufferU16(m_totalElements));
		assert(m_indicesBuffer != nullptr);

		auto* indPtr = m_indicesBuffer->ptr();
		int c = 0;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j <= m_lineSplit; j++)
			{
				*indPtr++ = c++;
			}
			*indPtr++ = m_restartValue;
		}
		m_indicesBuffer->flush();
		m_indicesBuffer->sync();

		// array
		auto stat = res::try_create_vertex_array(m_patchArray);
		assert(stat);

		glBindVertexArray(m_patchArray.id);
		glEnableVertexAttribArray(0);
		glBindVertexBuffer(0, m_vertexBuffer->id(), 0, sizeof(GfxSBuffer3::vec));
		glVertexBindingDivisor(0, 0);
		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer->id());

		// prog
		m_unicolorProg.reset(new ProgUnicolorPrim3D());
		assert(m_unicolorProg != nullptr);

		// wait sync
		m_indicesBuffer->waitSync();
	}

	void deinitGfx()
	{
		m_unicolorProg.reset();
		m_patchArray.reset();
		m_vertexBuffer.reset();
		m_indicesBuffer.reset();
	}

	void initState()
	{
		resetState();
		m_patchChanged = true;
	}

	void deinitState()
	{
		resetState();
	}

private: // action
	AppAction handleState()
	{
		if (m_needGoBack)
		{
			m_needGoBack = false;
			
			return AppAction{ActionType::Pop};
		}

		if (m_patchChanged)
		{
			updatePatch();

			m_changedIndex = invalid;
			m_patchChanged = false;
		}

		return {};
	}


private: // gui
	void doGui()
	{
		m_gui->draw();
	}

	void onVertexChanged(u32 index)
	{
		m_changedIndex = index;
		m_patchChanged = true;
	}

	void onReturnBack()
	{
		m_needGoBack = true;
	}


private: // render
	void render()
	{
		m_vertexBuffer->waitSync();

		auto [w, h] = m_window->framebufferSize();
		glViewport(0, 0, w, h);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		m_unicolorProg->use();
		m_unicolorProg->setProj(projMat());
		m_unicolorProg->setPrimColor(m_color0);

		auto restartEnabled = glIsEnabled(GL_PRIMITIVE_RESTART);
		if (restartEnabled == GL_FALSE)
			glEnable(GL_PRIMITIVE_RESTART);

		glPrimitiveRestartIndex(m_restartValue);
		glBindVertexArray(m_patchArray.id);
		glDrawElements(GL_LINE_STRIP, m_totalElements, GL_UNSIGNED_SHORT, (void*)m_indicesOffset);

		if (restartEnabled == GL_FALSE)
			glDisable(GL_PRIMITIVE_RESTART);

		m_vertexBuffer->sync();
	}


private: // utility
	mat4 projMat()
	{
		auto [w, h] = m_window->framebufferSize();
		mat4 proj = perspective(radians(45.0f), (f32)w / h, 0.1f, 100.0f);
		mat4 view = lookAt(vec3(3.0), vec3(0.0), vec3(0.0, 1.0, 0.0));
		return proj * view;
	}

private: // operation
	void resetState()
	{
		m_changedIndex = invalid;
		m_patchChanged = false;
		m_needGoBack    = false;
	}

	void updatePatch()
	{
		m_vertexBuffer->waitSync();

		auto* verts = m_vertexBuffer->ptr();

		// u-direction
		for (int i = 0; i <= 3; i++)
		{
			for (int j = 0; j <= m_lineSplit; j++)
			{
				*verts++ = m_patch.eval((f32)j / m_lineSplit, (f32)i / 3);
			}
		}
		// v-direction
		for (int i = 0; i <= 3; i++)
		{
			for (int j = 0; j <= m_lineSplit; j++)
			{
				*verts++ = m_patch.eval((f32)i / 3, (f32)j / m_lineSplit);
			}
		}

		m_vertexBuffer->flush();
		m_vertexBuffer->sync();
	}


private:
	// app
	MainWindow* m_window{};
	bool m_initialized{};

	// gui
	std::unique_ptr<Lab7Gui> m_gui;
	sig::Connection m_vertexChangedConn;
	sig::Connection m_returnBackConn;

	// gfx
	u32 m_totalVertices{};
	u32 m_totalElements{};
	u32 m_indicesOffset{};
	u16 m_restartValue{};
	u32 m_lineSplit{};
	res::VertexArray m_patchArray; // primitve: GL_LINE_STRIP, index type: GL_UNSIGNED_BYTE
	std::unique_ptr<GfxSBuffer3>   m_vertexBuffer;
	std::unique_ptr<GfxSBufferU16> m_indicesBuffer;
	std::unique_ptr<ProgUnicolorPrim3D> m_unicolorProg;
	vec4 m_color0{0.0, 0.0, 0.0, 1.0};

	// patch
	bezier::Patch3D<vec3> m_patch;

	// state
	u32 m_changedIndex{}; // not used
	bool m_patchChanged{};
	bool m_needGoBack{};
};

REGISTER_STATE(lab7, Lab7)
