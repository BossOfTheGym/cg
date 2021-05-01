#include "lab1.h"
#include "lab1-gui.h"

#include "app.h"
#include "quadtree.h"
#include "primitive.h"
#include "prog-rect.h"
#include "prog-frame.h"
#include "main-window.h"
#include "prog-color-prim.h"
#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include "gfx-res.h"

#include <vector>
#include <random>
#include <cassert>
#include <iostream>

using namespace prim;

namespace
{
	constexpr const u32 max_points = 1'000'000;

	using GfxDBuffer2 = GfxSDBuffer<vec2>;
	using GfxDBuffer3 = GfxSDBuffer<vec3>;
	using GfxDBuffer4 = GfxSDBuffer<vec4>;

	template<class vec1, class vec2, class size>
	void store_vec_data(vec1* ptr, vec2* data, size count)
	{
		for (size i = 0; i < count; i++)
			ptr[i] = data[i];
	}

	template<class vec1, class vec2, class size>
	void store_vec_value(vec1* ptr, vec2 value, size count)
	{
		for (size i = 0; i < count; i++)
			ptr[i] = value;
	}

	template<class vec1, class vec2, class index>
	void store_vec_value(vec1* ptr, vec2 value, index* indices, index indexCount)
	{
		for (index i = 0; i < indexCount; i++)
			ptr[indices[i]] = value;
	}

	template<class Handle>
	class DQuery
	{
	public:
		using Query = std::vector<Handle>;

	public:
		DQuery(u32 initCapacity)
		{
			m_back.reserve(initCapacity);
			m_front.reserve(initCapacity);
		}

	public:
		void swap()
		{
			std::swap(m_back, m_front);
		}

		Query& back()
		{
			return m_back;
		}

		Query& front()
		{
			return m_front;
		}

		void clear()
		{
			m_back.clear();
			m_front.clear();
		}

	private:
		Query m_back;
		Query m_front;
	};


	class Lab1Impl
	{
		friend class Sampler;

	public:
		class Sampler
		{
			friend class Lab1Impl;

			Sampler(Lab1Impl* host) : m_host(host) {}

		public:
			Sampler() = default;

			const Vec2& operator() (u32 handle) const
			{
				return m_host->m_points[handle];
			}

		private:
			Lab1Impl* m_host{nullptr};
		};

		using Handle = u32;
		using TreeHelper = qtree::Helper<Handle, Sampler, qtree::Allocator>;
		using Position      = typename TreeHelper::Position;
		using NodeAllocator = typename TreeHelper::NodeAllocator;
		using QuadTree      = typename TreeHelper::Tree;


	public: // cnst & dstr
		Lab1Impl(Lab1* lab) 
			: m_lab(lab)
			, m_window(lab->app().window())
		{}

		~Lab1Impl()
		{
			deinit();
		}

	public: // app state
		bool init()
		{
			if (m_initialized)
				return true;

			initGui();
			initInput();
			initGfx();
			initPoints();
			initState();
			
			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			deinitGfx();
			deinitInput();
			deinitGui();
			deinitPoints();
			deinitState();

			m_paused = true;

			m_initialized = false;
		}

		AppAction execute()
		{
			doGui();
			swapDBuffered();
			render();
			return handleState();
		}

		void pause()
		{
			if (!m_paused)
			{
				deinitInput();
				m_paused = true;
			}
		}

		void resume()
		{
			if (m_paused)
			{
				initInput();
				m_paused = false;
			}
		}

	private: // init & deinit
		void initGui()
		{
			auto [w, h] = m_window->framebufferSize();
			f32 x0 = 0.2 * w;
			f32 x1 = 0.8 * w;
			f32 y0 = 0.2 * h;
			f32 y1 = 0.8 * h;

			m_gui.reset(new Lab1Gui(max_points, 0u, x0, x1, y0, y1));
			assert(m_gui != nullptr);

			m_generatePointsConn = m_gui->generatePoints    .connect([&](u32 pts){ onGeneratePoints(pts); });
			m_goBackConn         = m_gui->returnBack        .connect([&](){ onReturnBack(); });
			m_frameChangedConn   = m_gui->frameParamsChanged.connect([&](f32 x0, f32 x1, f32 y0, f32 y1){ onFrameParamsChanged(x0, x1, y0, y1); });
			m_frameParams = {x0, x1, y0, y1};
		}

		void deinitGui()
		{
			m_generatePointsConn.release();
			m_goBackConn.release();
			m_frameChangedConn.release();
			m_gui.reset();
		}

		void initInput()
		{
			m_mouseButtonConn = m_window->mouseButton.connect( [&](int button, int action, int mods){ onMouseButton(button, action, mods); });
			m_mouseMovedConn  = m_window->mouseMoved .connect( [&](double x, double y){ onMouseMoved(x, y); });
		}

		void deinitInput()
		{
			m_mouseButtonConn.release();
			m_mouseMovedConn.release();
		}

		void initGfx()
		{
			auto [w, h] = m_window->framebufferSize();

			m_gfxPoints.reset(new GfxDBuffer2(max_points));
			assert(m_gfxPoints != nullptr);
			m_gfxColors.reset(new GfxDBuffer4(max_points));
			assert(m_gfxColors != nullptr);

			m_progColorPrim.reset(new ProgColorPrim());
			assert(m_progColorPrim != nullptr);
			m_progFrame.reset(new ProgFrame());
			assert(m_progFrame != nullptr);
			m_progRect.reset(new ProgRect());
			assert(m_progRect != nullptr);

			bool stat = res::try_create_texture(m_attachment); assert(stat);
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA8, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stat = res::try_create_framebuffer(m_renderedPoints); assert(stat);
			glNamedFramebufferTexture(m_renderedPoints.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_renderedPoints.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_renderedPoints.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			stat = res::try_create_vertex_array(m_dummyArray); assert(stat);

			m_vertexArray.reset(new DVertexArray(0u));
			assert(m_vertexArray != nullptr);

			auto arrayId = m_vertexArray->id();
			glEnableVertexArrayAttrib(arrayId, 0);
			glVertexArrayVertexBuffer(arrayId, 0, m_gfxPoints->id(), 0, sizeof(GfxDBuffer2::vec));
			glVertexArrayBindingDivisor(arrayId, 0, 0);
			glVertexArrayAttribFormat(arrayId, 0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(arrayId, 0, 0);
			
			glEnableVertexArrayAttrib(arrayId, 1);
			glVertexArrayVertexBuffer(arrayId, 1, m_gfxColors->id(), 0, sizeof(GfxDBuffer4::vec));
			glVertexArrayBindingDivisor(arrayId, 1, 0);
			glVertexArrayAttribFormat(arrayId, 1, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(arrayId, 1, 1);
		}

		void deinitGfx()
		{
			m_vertexArray.reset();
			m_dummyArray.reset();

			m_renderedPoints.reset();
			m_attachment.reset();

			m_progRect.reset();
			m_progFrame.reset();
			m_progColorPrim.reset();

			m_gfxColors.reset();
			m_gfxPoints.reset();
		}

		void initPoints()
		{
			auto [w, h] = m_window->framebufferSize();

			m_points.reserve(max_points);
			m_query.reset(new DQuery<Handle>(max_points));
			m_tree.reset(new QuadTree({{0, 0}, {w, h}}, Sampler(this), NodeAllocator()));
		}

		void deinitPoints()
		{
			m_tree.reset();
			m_query.reset();
			m_points.clear();
		}

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
			if (m_needGenerate)
				m_frameChanged = false;

			if (m_goBack)
			{
				m_goBack = false;

				return AppAction{ActionType::Pop}; // we need to pop current state
			}

			if (m_frameChanged)
			{
				m_frameChanged = false;

				doQuery(m_frameParams);
			}

			if (m_needGenerate)
			{
				m_needGenerate = false;

				generatePoints();
			}

			return {}; // do nothing
		}


	private: // gui callbacks
		void doGui()
		{
			m_gui->draw();
		}

		void onGeneratePoints(u32 amount)
		{
			m_pointsGenerated = amount;

			m_needGenerate = true;
		}

		void onReturnBack()
		{
			m_goBack = true;
		}

		void onFrameParamsChanged(f32 x0, f32 x1, f32 y0, f32 y1)
		{
			m_frameParams[0] = x0;
			m_frameParams[1] = x1;
			m_frameParams[2] = y0;
			m_frameParams[3] = y1;
			
			m_frameChanged = true;
		}


	private: // input callbacks
		void onMouseButton(int button, int action, int mods)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				auto [x, y] = m_window->cursorPos();
				if (!m_frameCaptured && action == GLFW_PRESS)
				{ 
					if (inFrame(x, y))
					{
						m_capturedX = x;
						m_capturedY = y;
						m_framePrev = m_frameParams;

						m_frameCaptured = true;
					}
				}
				if (m_frameCaptured && action == GLFW_RELEASE)
					m_frameCaptured = false;
			}
		}

		void onMouseMoved(double xpos, double ypos)
		{
			if (m_frameCaptured)
			{
				double dx = xpos - m_capturedX;
				double dy = ypos - m_capturedY;

				m_frameParams[0] = m_framePrev[0] + dx;
				m_frameParams[1] = m_framePrev[1] + dx;
				m_frameParams[2] = m_framePrev[2] + dy;
				m_frameParams[3] = m_framePrev[3] + dy;
				m_gui->setFrameParams(m_frameParams[0], m_frameParams[1], m_frameParams[2], m_frameParams[3]);

				m_frameChanged = true;
			}
		}


	private: // utility
		bool inFrame(double x, double y)
		{
			return m_frameParams[0] <= x && x <= m_frameParams[1] && m_frameParams[2] <= y && y <= m_frameParams[3];
		}

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

		AABB2 frameToAABB()
		{
			return {{m_frameParams[0], m_frameParams[2]}, {m_frameParams[1], m_frameParams[3]}};
		}

		void resetState()
		{
			m_frameCaptured = false;
			m_frameChanged = false;

			m_initialized = false;
			m_needRedraw = false;
			m_needGenerate = false;
			m_needSwap = false;
			m_paused = false;
			m_goBack = false;
		}


	private: // generation
		void generatePoints()
		{
			auto [w, h] = m_window->framebufferSize();

			// points
			auto seed = std::random_device()();
			std::minstd_rand0 base(seed);
			std::uniform_real_distribution<Float> genX(0.02 * w, 0.98 * w);
			std::uniform_real_distribution<Float> genY(0.02 * h, 0.98 * h);

			m_points.clear();
			for (u32 i = 0; i < m_pointsGenerated; i++)
				m_points.push_back(Vec2{genX(base), genY(base)});

			// vertices
			m_gfxPoints->resize(m_pointsGenerated);
			store_vec_data(m_gfxPoints->frontPtr(), m_points.data(), m_pointsGenerated);
			store_vec_data(m_gfxPoints->backPtr() , m_points.data(), m_pointsGenerated);
			m_gfxPoints->flush();
			m_gfxPoints->sync();

			// colors
			m_gfxColors->resize(m_pointsGenerated);
			store_vec_value(m_gfxColors->frontPtr(), m_color0, m_pointsGenerated);
			store_vec_value(m_gfxColors->backPtr() , m_color0, m_pointsGenerated);
			m_gfxColors->flush();
			m_gfxColors->sync();

			// vertex array
			m_vertexArray->primitives(m_pointsGenerated);

			// quadtree & query
			m_query->clear();
			m_tree->clear();

			for (u32 i = 0; i < m_pointsGenerated; i++)
				m_tree->insert(i);

			m_frameChanged = true;
		}


	private: // query
		void doQuery(const vec4& params)
		{
			m_gfxColors->waitSyncBack();

			auto& query = m_query->back();

			// reset color
			store_vec_value(m_gfxColors->backPtr(), m_color0, query.data(), (Handle)query.size());

			// query points
			m_tree->query(frameToAABB(), query);

			// set new color
			store_vec_value(m_gfxColors->backPtr(), m_color1, query.data(), (Handle)query.size());

			m_gfxColors->flushBack();
			m_gfxColors->syncBack();

			// data changed so we will need to swap next frame
			m_needSwap   = true;
			m_needRedraw = true;
		}


	private: // double-buffer control
		void swapDBuffered()
		{
			if (m_needSwap)
			{
				m_needSwap = false;

				m_gfxColors->swap();
				m_gfxPoints->swap();
				m_vertexArray->swap();
				m_query->swap();
			}
		}


	private: // rendering
		void render()
		{
			renderPoints();
			renderAttachment();
			renderFrame();
		}

		void renderPoints()
		{
			if (!m_needRedraw)
				return;
			m_needRedraw = false;

			m_gfxColors->waitSyncFront();

			auto [w, h] = m_window->framebufferSize();

			GLint64 bound{};
			glGetInteger64v(GL_DRAW_FRAMEBUFFER_BINDING, &bound);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_renderedPoints.id);
			glViewport(0, 0, w, h);
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_progColorPrim->use();
			m_progColorPrim->setProj(projMat());
			
			glBindVertexArray(m_vertexArray->id());
			glDrawArrays(GL_POINTS, m_vertexArray->front(), m_vertexArray->primitives());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bound);
		}

		void renderAttachment()
		{
			glBindVertexArray(m_dummyArray.id);
			m_progRect->use();
			m_progRect->setTexture(m_attachment.id);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		void renderFrame()
		{
			m_progFrame->use();
			m_progFrame->setFrameColor(m_color2);
			m_progFrame->setFrameParams(m_frameParams);
			m_progFrame->setProj(projMat());

			glBindVertexArray(m_dummyArray.id);
			glDrawArrays(GL_LINE_LOOP, 0, 4);
		}


	private:
		// app state
		Lab1*       m_lab{};
		MainWindow* m_window{};		
		bool m_initialized{false};

		// gui
		std::unique_ptr<Lab1Gui> m_gui;
		sig::Connection m_goBackConn;
		sig::Connection m_frameChangedConn;
		sig::Connection m_generatePointsConn;
		
		// input
		sig::Connection m_mouseButtonConn;
		sig::Connection m_mouseMovedConn;
		
		// gfx
		res::VertexArray m_dummyArray;
		res::Framebuffer m_renderedPoints;
		res::Texture     m_attachment;
		std::unique_ptr<ProgColorPrim> m_progColorPrim;
		std::unique_ptr<ProgRect>      m_progRect;
		std::unique_ptr<ProgFrame>     m_progFrame;
		std::unique_ptr<DVertexArray> m_vertexArray;
		std::unique_ptr<GfxDBuffer2>  m_gfxPoints;
		std::unique_ptr<GfxDBuffer4>  m_gfxColors;
		vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		vec4 m_color1{1.0, 0.0, 0.0, 1.0};
		vec4 m_color2{0.0, 0.0, 1.0, 1.0};

		// state
		vec4 m_frameParams{};
		vec4 m_framePrev{};

		u32 m_pointsGenerated{};

		double m_capturedX{};
		double m_capturedY{};

		bool m_frameCaptured{false};
		bool m_frameChanged{false};

		bool m_needGenerate{false};
		bool m_needRedraw{false};
		bool m_needSwap{false};
		bool m_paused{false};
		bool m_goBack{false};

		// points
		std::vector<Vec2> m_points;
		std::unique_ptr<DQuery<Handle>> m_query;
		std::unique_ptr<QuadTree>       m_tree;
	};
}


Lab1::Lab1(App* app) : AppState(app)
{
	m_impl.reset(new Lab1Impl(this));
	assert(m_impl != nullptr);
}

Lab1::~Lab1()
{
	deinit();

	m_impl.reset();
}

bool Lab1::init()
{
	return m_impl->init();
}

void Lab1::deinit()
{
	m_impl->deinit();
}

AppAction Lab1::execute()
{
	return m_impl->execute();
}

void Lab1::pause()
{}

void Lab1::resume()
{}
