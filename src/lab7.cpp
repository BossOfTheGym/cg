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

#include <iostream>

using namespace prim;

using GfxSBuffer3   = GfxSBuffer<vec3>;
using GfxSBufferU16 = GfxSBuffer<u16>;

namespace
{
	constexpr u32 show_every  = 6u;
	constexpr u32 patch_split = 48u; // multiple of show_every 
	static_assert(patch_split % show_every == 0, "patch_split must be multiple of show_every.");

	constexpr u32 invalid = std::numeric_limits<u32>::max();

	// https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
	struct Trackball
	{		
		static f32 z(vec2 v, f32 r)
		{
			f32 rr = r * r;
			f32 vv = dot(v, v);
			if (vv <= rr / 2.0)
				return std::sqrt(rr - vv);
			return rr / 2 / std::sqrt(vv);
		}

		void track(vec2 v0, vec2 v1, f32 r)
		{
			vec3 r1 = normalize(vec3(v0, z(v0, r)));
			vec3 r2 = normalize(vec3(v1, z(v1, r)));
			vec3 n = cross(r1, r2);

			f32 nl = length(n);
			if (nl <= epsilon<f32>())
				return;
			n /= nl;

			f32 angle = std::acos(clamp(dot(r1, r2), -1.0f, +1.0f));

			rotate(rot,0.0f, vec3());
			rot = angleAxis(angle, n) * rot;
		}

		quat rot;
	};
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
		initInput();
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
		deinitInput();
		deinitGui();

		m_initialized = false;
	}

	virtual AppAction execute() override
	{
		doGui();
		handleInput();
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

	void initInput()
	{
		auto [x, y] = m_window->cursorPos();
		m_xPrev = x;
		m_yPrev = y;
		m_dragging = false;

		m_mouseClickedConn = m_window->mouseButton.connect([&](int button, int action, int mods){ onMouseClicked(button, action, mods); });
		m_mouseMovedConn   = m_window->mouseMoved.connect([&](double x, double y){ onMouseMoved(x, y); });
	}

	void deinitInput()
	{
		m_mouseClickedConn.release();
		m_mouseMovedConn.release();
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
		m_showEvery = show_every;
		m_totalVertices = 2 * (m_lineSplit / m_showEvery + 1) * (m_lineSplit + 1);
		m_totalElements = 2 * (m_lineSplit / m_showEvery + 1) * (m_lineSplit + 1 + 1);
		m_indicesOffset = 0;
		m_restartValue = (u16)(-1);

		m_vertexBuffer.reset(new GfxSBuffer3(m_totalVertices));
		assert(m_vertexBuffer != nullptr);

		m_indicesBuffer.reset(new GfxSBufferU16(m_totalElements));
		assert(m_indicesBuffer != nullptr);

		auto* indPtr = m_indicesBuffer->ptr();
		int c = 0;
		for (int i = 0; i < 2 * (m_lineSplit / m_showEvery + 1); i++)
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


private: // input
	void onMouseMoved(double x, double y)
	{
		m_xCurr = x;
		m_yCurr = y;
	}

	void onMouseClicked(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (!m_dragging && action == GLFW_PRESS)
				m_dragging = true;
			if (m_dragging && action == GLFW_RELEASE)
				m_dragging = false;	
		}
	}

	void handleInput()
	{
		if (m_dragging)
		{			
			auto [w, h] = m_window->framebufferSize();
			vec2 v0 = vec2(m_xPrev - (f32)w / 2, -m_yPrev + (f32)h / 2);
			vec2 v1 = vec2(m_xCurr - (f32)w / 2, -m_yCurr + (f32)h / 2);
			m_track.track(v0, v1, std::min((f32)w / 2, (f32)h / 2));
		}
		m_xPrev = m_xCurr;
		m_yPrev = m_yCurr;
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
		m_unicolorProg->setProj(transform());
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
	mat4 transform()
	{
		auto [w, h] = m_window->framebufferSize();
		h = std::max(1, h); // minimization workaround

		mat4 proj = perspective(radians(45.0f), (f32)w / h, 0.1f, 100.0f);
		mat4 view = lookAt(vec3(3.0), vec3(0.0), vec3(0.0, 1.0, 0.0));
		mat4 model = mat4(1.0);

		quat rot = m_track.rot;		
		vec4 xyz = vec4(rot.x, rot.y, rot.z, 0.0);
		rot.x = dot(view[0], xyz);
		rot.y = dot(view[1], xyz);
		rot.z = dot(view[2], xyz);

		mat4 track = toMat4(rot);
		return proj * view * track * model;
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
		for (int i = 0; i <= m_lineSplit / m_showEvery; i++)
		{
			for (int j = 0; j <= m_lineSplit; j++)
			{
				*verts++ = m_patch.eval((f32)j / m_lineSplit, (f32)i / (m_lineSplit / m_showEvery));
			}
		}
		// v-direction
		for (int i = 0; i <= m_lineSplit / m_showEvery; i++)
		{
			for (int j = 0; j <= m_lineSplit; j++)
			{
				*verts++ = m_patch.eval((f32)i / (m_lineSplit / m_showEvery), (f32)j / m_lineSplit);
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

	// input
	Trackball m_track{quat(1.0, 0.0, 0.0, 0.0)};
	sig::Connection m_mouseMovedConn;
	sig::Connection m_mouseClickedConn;
	double m_xPrev{};
	double m_xCurr{};
	double m_yPrev{};
	double m_yCurr{};
	bool m_dragging{};

	// gfx
	u32 m_totalVertices{};
	u32 m_totalElements{};
	u32 m_indicesOffset{};
	u16 m_restartValue{};
	u32 m_showEvery{};
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
