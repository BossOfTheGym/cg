#include "lab6.h"
#include "lab6-gui.h"

#include "app.h"
#include "bezier.h"
#include "primitive.h"
#include "prog-frame.h"
#include "main-window.h"
#include "prog-unicolor-prim.h"
#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include "gfx-res.h"

#include <cmath>
#include <memory>
#include <random>
#include <numeric>
#include <cassert>
#include <utility>
#include <iostream>


using namespace prim;

namespace
{
	constexpr u32 invalid = std::numeric_limits<u32>::max();
	constexpr u32 patch_split = 100u;

	constexpr f32 frame_size = 10.0f;

	using GfxSBuffer2 = GfxSBuffer<vec2>;
	using Patch = bezier::Patch2D<vec2>;

	class Lab6Impl
	{
	public:
		Lab6Impl(Lab6* lab) : m_lab(lab), m_window(lab->app().window())
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

			initGui();
			initInput();
			initPatch();
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
			deinitPatch();
			deinitInput();
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
			m_gui.reset(new Lab6Gui());
			assert(m_gui != nullptr);
			m_backConn = m_gui->back.connect([&](){ onBack(); });
		}

		void deinitGui()
		{
			m_backConn.release();
			m_gui.reset();
		}

		void initInput()
		{
			m_mouseButtonConn = m_window->mouseButton.connect([&](int button, int action, int mods){ onMouseButton(button, action, mods); });
			m_mouseMovedConn  = m_window->mouseMoved .connect([&](double x, double y){ onMouseMove(x, y); });
		}

		void deinitInput()
		{
			m_mouseButtonConn.release();
			m_mouseMovedConn.release();
		}

		void initGfx()
		{
			bool stat{};

			// dummy
			stat = res::try_create_vertex_array(m_dummyArray);
			assert(stat);

			// bezier buffer
			m_bezierBuffer.reset(new GfxSBuffer2(patch_split + 1));
			assert(m_bezierBuffer != nullptr);

			// bezier array
			stat = res::try_create_vertex_array(m_bezierArray);
			assert(stat);
			glBindVertexArray(m_bezierArray.id);
			glEnableVertexAttribArray(0);
			glBindVertexBuffer(0, m_bezierBuffer->id(), 0, sizeof(GfxSBuffer2::vec));
			glVertexBindingDivisor(0, 0);
			glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(0, 0);

			// progs
			m_progFrame.reset(new ProgFrame());
			assert(m_progFrame != nullptr);
			m_progUnicolor.reset(new ProgUnicolorPrim());
			assert(m_progUnicolor);
		}

		void deinitGfx()
		{
			m_progUnicolor.reset();
			m_progFrame.reset();

			m_bezierArray.reset();
			m_bezierBuffer.reset();

			m_dummyArray.reset();
		}

		void initPatch()
		{
			resetPatch();
		}

		void deinitPatch()
		{}

		void initState()
		{
			resetState();

			m_needUpdatePatch = true;
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

			if (m_needUpdatePatch)
			{
				m_needUpdatePatch = false;

				updatePatch();
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


	private: // input
		void onMouseButton(int button, int action, int mods)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				auto [x, y] = m_window->cursorPos();
				if (!m_pointCaptured && action == GLFW_PRESS)
				{
					m_capturedIndex = capturePatchPoint({x, y});
					if (m_capturedIndex != invalid)
						m_pointCaptured = true;
				}

				if (m_pointCaptured && action == GLFW_RELEASE)
					m_pointCaptured = false;
			}
		}

		void onMouseMove(double x, double y)
		{
			if (m_pointCaptured)
			{
				setPatchPoint(m_capturedIndex, {x, y});

				m_needUpdatePatch = true;
			}
		}


	private: // utility
		mat4 projMat()
		{
			// projects shitty top-left originated window into OpenGL coordinate system
			auto [w, h] = m_window->framebufferSize();

			mat4 proj{1.0};
			proj[0][0] = +2.0 / w; 
			proj[1][1] = -2.0 / h;
			proj[3][0] = -1.0;
			proj[3][1] = +1.0;
			return proj;
		}

		vec4 frameParams(const vec2& vec)
		{
			return vec4(vec.x - frame_size, vec.x + frame_size, vec.y - frame_size, vec.y + frame_size);
		}

		bool inFrame(const vec4& frame, const vec2& vec)
		{
			return frame[0] <= vec.x && vec.x <= frame[1] 
				&& frame[2] <= vec.y && vec.y <= frame[3];
		}

		void resetState()
		{
			m_capturedIndex = invalid;
			m_needUpdatePatch = false;
			m_pointCaptured = false;
			m_mouseClicked = false;
			m_mouseMoved = false;
			m_needGoBack = false;
		}

		void resetPatch()
		{
			auto [w, h] = m_window->framebufferSize();
			m_patch.v0 = Vec2(0.2 * w, 0.5 * h);
			m_patch.v1 = Vec2(0.4 * w, 0.5 * h);
			m_patch.v2 = Vec2(0.6 * w, 0.5 * h);
			m_patch.v3 = Vec2(0.8 * w, 0.5 * h);
		}


	private: // operation
		void updatePatch()
		{
			m_bezierBuffer->waitSync();

			auto ptr = m_bezierBuffer->ptr();
			auto dt = 1.0 / patch_split;
			for (u32 i = 0; i <= patch_split; i++)
				ptr[i] = bezier::eval2d(m_patch, i * dt);

			m_bezierBuffer->flush();
			m_bezierBuffer->sync();
		}

		u32 capturePatchPoint(const vec2& vec)
		{
			if (inFrame(frameParams(m_patch.v0), vec))
				return 0u;
			if (inFrame(frameParams(m_patch.v1), vec))
				return 1u;
			if (inFrame(frameParams(m_patch.v2), vec))
				return 2u;
			if (inFrame(frameParams(m_patch.v3), vec))
				return 3u;
			return invalid;
		}

		void setPatchPoint(u32 ind, const vec2& vec)
		{
			switch(ind)
			{
				case 0u:
					m_patch.v0 = vec;
					break;
				case 1u:
					m_patch.v1 = vec;
					break;
				case 2u:
					m_patch.v2 = vec;
					break;
				case 3u:
					m_patch.v3 = vec;
					break;
			}
		}

	private: // render
		void render()
		{
			m_bezierBuffer->waitSync();

			auto [w, h] = m_window->framebufferSize();
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glViewport(0, 0, w, h);

			glBindVertexArray(m_bezierArray.id);
			m_progUnicolor->use();
			m_progUnicolor->setPrimColor(m_color0);
			m_progUnicolor->setProj(projMat());
			glDrawArrays(GL_LINE_STRIP, 0, patch_split + 1);

			glBindVertexArray(m_dummyArray.id);
			m_progFrame->use();			
			m_progFrame->setProj(projMat());
			m_progFrame->setFrameColor(m_color1);
			m_progFrame->setFrameParams(frameParams(m_patch.v0));
			glDrawArrays(GL_LINE_LOOP, 0, 4);
			m_progFrame->setFrameParams(frameParams(m_patch.v1));
			glDrawArrays(GL_LINE_LOOP, 0, 4);
			m_progFrame->setFrameParams(frameParams(m_patch.v2));
			glDrawArrays(GL_LINE_LOOP, 0, 4);
			m_progFrame->setFrameParams(frameParams(m_patch.v3));
			glDrawArrays(GL_LINE_LOOP, 0, 4);

			m_bezierBuffer->sync();
		}


	private:
		// app state
		Lab6* m_lab{};
		MainWindow* m_window{};
		bool m_initialized{false};

		// gui state
		std::unique_ptr<Lab6Gui> m_gui;
		sig::Connection m_backConn;

		// input state
		sig::Connection m_mouseButtonConn;
		sig::Connection m_mouseMovedConn;

		// patch
		Patch m_patch;

		// gfx
		res::VertexArray m_dummyArray;
		res::VertexArray m_bezierArray; // primitive: GL_LINE_STRIP, non-indexed
		std::unique_ptr<GfxSBuffer2> m_bezierBuffer;
		std::unique_ptr<ProgUnicolorPrim> m_progUnicolor;
		std::unique_ptr<ProgFrame>        m_progFrame;
		vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		vec4 m_color1{1.0, 0.0, 0.0, 1.0};

		// state
		u32 m_capturedIndex{};
		bool m_needUpdatePatch{false};
		bool m_pointCaptured{false};
		bool m_mouseClicked{false};
		bool m_mouseMoved{false};
		bool m_needGoBack{false};
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
