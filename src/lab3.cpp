#include "lab3.h"
#include "lab3-gui.h"

#include "app.h"
#include "section.h"
#include "primitive.h"
#include "prog-rect.h"
#include "main-window.h"
#include "convex_hull.h"
#include "prog-color-prim.h"
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
	constexpr const u32 max_points = 1'000'000;
	constexpr const Float pi = 3.14159265358979323846;

	using GfxSBuffer2   = GfxSBuffer<vec2>;
	using GfxSBuffer3   = GfxSBuffer<vec3>;
	using GfxSBuffer4   = GfxSBuffer<vec4>;
	using GfxSBufferU32 = GfxSBuffer<u32>;

	using Handle = u32;

	class Lab3Impl
	{
	public:
		Lab3Impl(Lab3* lab) : m_lab(lab), m_window(lab->app().window())
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

			initGui();
			initGfx();
			initGen();
			initPoints();
			initState();

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			deinitState();
			deinitPoints();
			deinitGen();
			deinitGfx();
			deinitGui();

			m_gui.reset();
		}

		AppAction execute()
		{
			doGui();
			auto action = handleState();
			render();
			return action;
		}

	private: // init & deinit
		void initGui()
		{
			m_gui.reset(new Lab3Gui(max_points, 0u));
			assert(m_gui != nullptr);
			m_generateConn = m_gui->generate.connect([&](u32 count){ onGenerate(count); });
			m_buildConn    = m_gui->build   .connect([&](){ onBuild(); });
			m_clearConn    = m_gui->clear   .connect([&](){ onClear(); });
			m_backConn     = m_gui->back    .connect([&](){ onBack(); });
		}

		void deinitGui()
		{
			m_generateConn.release();
			m_buildConn.release();
			m_clearConn.release();
			m_backConn.release();
		}

		void initGfx()
		{
			bool stat{};

			auto [w, h] = m_window->framebufferSize();

			// dummy array
			stat = res::try_create_vertex_array(m_dummyArray);
			assert(stat);

			// buffers
			m_gfxVertices.reset(new GfxSBuffer2(max_points));
			assert(m_gfxVertices != nullptr);
			m_gfxIndices.reset(new GfxSBufferU32(max_points));
			assert(m_gfxIndices != nullptr);

			// arrays
			stat = res::try_create_vertex_array(m_pointsArray);
			assert(stat);
			glBindVertexArray(m_pointsArray.id);
			glEnableVertexAttribArray(0);
			glBindVertexBuffer(0, m_gfxVertices->id(), 0, sizeof(GfxSBuffer2::vec));
			glVertexBindingDivisor(0, 0);
			glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(0, 0);

			stat = res::try_create_vertex_array(m_hullArray);
			assert(stat);
			glBindVertexArray(m_hullArray.id);
			glEnableVertexAttribArray(0);
			glBindVertexBuffer(0, m_gfxVertices->id(), 0, sizeof(GfxSBuffer2::vec));
			glVertexBindingDivisor(0, 0);
			glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(0, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gfxIndices->id());

			// frame & attachment
			stat = res::try_create_texture(m_attachment);
			assert(stat);
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA8, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stat = res::try_create_framebuffer(m_frame); 
			assert(stat);
			glNamedFramebufferTexture(m_frame.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_frame.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_frame.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			// progs
			m_progRect.reset(new ProgRect());
			assert(m_progRect != nullptr);
			m_progUnicolorPrim.reset(new ProgUnicolorPrim());
			assert(m_progUnicolorPrim != nullptr);
		}

		void deinitGfx()
		{
			m_progUnicolorPrim.reset();
			m_progRect.reset();

			m_frame.reset();
			m_attachment.reset();

			m_hullArray.reset();
			m_pointsArray.reset();

			m_gfxIndices.reset();
			m_gfxVertices.reset();

			m_dummyArray.reset();
		}

		void initPoints()
		{
			m_points.reserve(max_points);
			m_pointHandles.reserve(max_points);
		}

		void deinitPoints()
		{
			m_points.clear();
			m_pointHandles.clear();
		}

		void initGen()
		{
			m_seed.seed(1);
		}

		void deinitGen()
		{}

		void initState()
		{
			resetState();

			m_needRedraw = true;
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

			if (m_needGenerate)
			{
				m_needGenerate = false;

				generatePoints();
			}

			if (m_needBuild)
			{
				m_needBuild = false;

				buildHull();
			}

			if (m_needClear)
			{
				m_needClear = false;
				
				clear();
			}

			return {};
		}


	private: // gui & gui callback
		void doGui()
		{
			m_gui->draw();
		}

		void onGenerate(u32 count)
		{
			m_pointsToGen = count;

			m_needGenerate = true;
		}

		void onBuild()
		{
			m_needBuild = true;
		}

		void onClear()
		{
			m_needClear = true;
		}

		void onBack()
		{
			m_needGoBack = true;
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

		void resetState()
		{
			m_needGenerate = false;
			m_needRedraw = false;
			m_needGoBack = false;
			m_needBuild = false;
			m_needClear = false;
			m_needSwap = false;
		}


	private: // operation
		void generatePoints()
		{
			auto [w, h] = m_window->framebufferSize();

			// points
			i32 cx = w / 2;
			i32 cy = h / 2;
			i32 A = 0.99 * w / 2;
			i32 B = 0.99 * h / 2;

			auto seed = m_seed();
			std::minstd_rand0 base(seed);
			std::uniform_real_distribution<Float> genA(0.0, 2 * pi);
			std::uniform_real_distribution<Float> genC(0.0, 1.0);

			m_points.clear();
			for (u32 i = 0; i < m_pointsToGen; i++)
			{
				auto a = genA(base);
				auto c = std::sqrt(genC(base));
				auto l = 0.7 + 0.3 * std::sin(10 * a);
				auto x = (i32)(c * A * l * std::cos(a) + cx);
				auto y = (i32)(c * B * l * std::sin(a) + cy);
				m_points.push_back(Vec2{x, y});
			}

			m_pointHandles.resize(m_pointsToGen);
			std::iota(m_pointHandles.begin(), m_pointHandles.end(), 0u);

			// vertices
			m_gfxVertices->waitSync();
			auto vertPtr = m_gfxVertices->ptr();
			for (u32 i = 0; i < m_points.size(); i++)
				vertPtr[i] = m_points[i];
			m_gfxVertices->flush(0u, m_points.size());
			m_gfxVertices->sync();

			// hull
			m_hullPoints = 0u;

			m_needRedraw = true;
		}

		void buildHull()
		{
			auto sampler = [&](auto handle)
			{
				assert(handle < m_points.size());

				return m_points[handle];
			};

			auto res = hull::convex_hull_graham(m_pointHandles, sampler);

			m_hullPoints = res.size();

			m_gfxIndices->waitSync();
			auto indPtr = m_gfxIndices->ptr();
			for (u32 i = 0; i < m_hullPoints; i++)
				indPtr[i] = res[i];
			m_gfxIndices->flush(0, m_hullPoints);
			m_gfxIndices->sync();

			m_gui->hullWasBuilt();

			m_needRedraw = true;
		}

		void clear()
		{
			m_hullPoints = 0u;
			m_points.clear();
			m_pointHandles.clear();

			m_needRedraw = true;
		}


	private: // render
		void render()
		{
			renderPointsHull();
			renderAttachment();
		}

		void renderPointsHull()
		{
			if (!m_needRedraw)
				return;
			m_needRedraw = false;

			m_gfxVertices->waitSync();
			m_gfxIndices->waitSync();

			auto [w, h] = m_window->framebufferSize();

			GLint64 bound{};
			glGetInteger64v(GL_DRAW_FRAMEBUFFER_BINDING, &bound);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frame.id);
			glViewport(0, 0, w, h);
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_progUnicolorPrim->use();
			m_progUnicolorPrim->setProj(projMat());

			m_progUnicolorPrim->setPrimColor(m_color1);
			glBindVertexArray(m_hullArray.id);
			glDrawElements(GL_LINE_LOOP, m_hullPoints, GL_UNSIGNED_INT, nullptr);

			m_progUnicolorPrim->setPrimColor(m_color0);
			glBindVertexArray(m_pointsArray.id);
			glDrawArrays(GL_POINTS, 0, m_points.size());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bound);

			m_gfxVertices->sync();
			m_gfxIndices->sync();
		}

		void renderAttachment()
		{
			glBindVertexArray(m_dummyArray.id);
			m_progRect->use();
			m_progRect->setTexture(m_attachment.id);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}


	private:
		// app state
		Lab3*       m_lab{};
		MainWindow* m_window{};
		bool m_initialized{false};

		// gui
		std::unique_ptr<Lab3Gui> m_gui;
		sig::Connection m_generateConn;
		sig::Connection m_buildConn;
		sig::Connection m_clearConn;
		sig::Connection m_backConn;

		// gfx
		res::VertexArray m_dummyArray;  // empty
		res::VertexArray m_pointsArray; // non-indexed, GL_POINTS
		res::VertexArray m_hullArray;   // indexed, GL_LINE_LOOP

		res::Framebuffer m_frame;
		res::Texture     m_attachment;

		std::unique_ptr<GfxSBuffer2>   m_gfxVertices;
		std::unique_ptr<GfxSBufferU32> m_gfxIndices;

		std::unique_ptr<ProgRect>         m_progRect;
		std::unique_ptr<ProgUnicolorPrim> m_progUnicolorPrim;
		
		vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		vec4 m_color1{1.0, 0.0, 0.0, 1.0};

		// state
		std::minstd_rand m_seed;
		u32 m_pointsToGen{};
		bool m_needGenerate{false};
		bool m_needRedraw{false};
		bool m_needGoBack{false};
		bool m_needBuild{false};
		bool m_needClear{false};
		bool m_needSwap{false}; // not used, I got lazy and didn't use double buffering(but the code would remain almost same)

		// points
		std::vector<Vec2> m_points;
		std::vector<u32>  m_pointHandles;
		u32 m_hullPoints{};
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
