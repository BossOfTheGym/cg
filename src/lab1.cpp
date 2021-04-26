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

#include <vector>
#include <random>
#include <cassert>
#include <iostream>

namespace
{
	constexpr const u32 max_points = 1'000'000;

	// TODO : explicit synchronization
	// TODO : set intial frameParams
	template<class vec_t> // vec2, vec3, vec4
	class GfxBuffer
	{
	public:
		using vec = vec_t;

	public:
		GfxBuffer(u32 initCapacity) : m_capacity(initCapacity)
		{
			bool stat{};

			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			stat = res::try_create_storage_buffer(m_buffer, initCapacity * sizeof(vec), flags, nullptr);
			assert(stat);

			flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			m_mappedPtr = (vec*)glMapNamedBufferRange(m_buffer.id, 0, initCapacity * sizeof(vec), flags);
			assert(m_mappedPtr != nullptr);
		}

		~GfxBuffer()
		{
			glUnmapNamedBuffer(m_buffer.id);
		}

	public:
		u32 capacity() const
		{
			return m_capacity;
		}

		res::Id id() const
		{
			return m_buffer.id;
		}	

		void waitSync()
		{
			if (m_sync.valid())
			{
				while (true)
				{
					GLenum result = glClientWaitSync(m_sync.id, GL_SYNC_FLUSH_COMMANDS_BIT, 25);
					if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
						break;
				}
				m_sync.del();
			}
		}

		void insertSync()
		{
			m_sync = res::create_fence_sync();
		}

		void write(const vec& v, u32 index)
		{
			assert(index < m_capacity);

			m_mappedPtr[index] = v;
		}

	private:
		res::Buffer    m_buffer{};
		res::FenceSync m_sync{};

		u32 m_capacity{};
		vec* m_mappedPtr{nullptr};
	};

	using GfxBuffer2 = GfxBuffer<prim::vec2>;
	using GfxBuffer3 = GfxBuffer<prim::vec3>;
	using GfxBuffer4 = GfxBuffer<prim::vec4>;

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

			const prim::Vec2& operator() (u32 handle) const
			{
				return m_host->m_points[handle];
			}

		private:
			Lab1Impl* m_host{nullptr};
		};

		using Handle = u32;
		using TreeHelper = qtree::Helper<Handle, Sampler, qtree::Allocator>;
		using QuadTree   = typename TreeHelper::Tree;

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

			m_initialized = false;
		}

		AppAction execute()
		{
			m_gui->draw();
			renderPoints();
			renderAttachment();
			renderFrame();

			if (m_goBack)
			{
				m_goBack = false;
				return AppAction{ActionType::Pop};
			}
			return {}; // Nothing
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

	private: // init functions
		void initGui()
		{
			auto [w, h] = m_window->framebufferSize();
			f32 x0 = 0.2 * w;
			f32 x1 = 0.8 * w;
			f32 y0 = 0.2 * h;
			f32 y1 = 0.8 * h;

			m_gui.reset(new Lab1Gui(max_points, 0u, x0, x1, y0, y1));
			assert(m_gui != nullptr);

			m_generatePointsConn = m_gui->generatePoints    .connect([&](u32 pts){ generatePoints(pts); });
			m_goBackConn         = m_gui->returnBack        .connect([&](){ returnBack(); });
			m_frameChangedConn   = m_gui->frameParamsChanged.connect([&](f32 x0, f32 x1, f32 y0, f32 y1) { frameParamsChanged(x0, x1, y0, y1); });
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
			m_mouseButtonConn = m_window->mouseButton.connect( [&](int button, int action, int mods){ mouseButton(button, action, mods); });
			m_mouseMovedConn  = m_window->mouseMoved .connect( [&](double x, double y){ mouseMoved(x, y); });
		}

		void deinitInput()
		{
			m_mouseButtonConn.release();
			m_mouseMovedConn.release();
		}

		void initGfx()
		{
			auto [w, h] = m_window->framebufferSize();

			m_gfxPoints.reset(new GfxBuffer2(max_points));
			assert(m_gfxPoints != nullptr);
			m_gfxColors.reset(new GfxBuffer4(max_points));
			assert(m_gfxColors != nullptr);

			m_progColorPrim.reset(new ProgColorPrim());
			assert(m_progColorPrim != nullptr);
			m_progFrame.reset(new ProgFrame());
			assert(m_progFrame != nullptr);
			m_progRect.reset(new ProgRect());
			assert(m_progRect != nullptr);

			bool stat = res::try_create_texture(m_attachment);
			assert(stat);
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA32F, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stat = res::try_create_framebuffer(m_renderedPoints);
			assert(stat);
			glNamedFramebufferTexture(m_renderedPoints.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_renderedPoints.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_renderedPoints.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			stat = res::try_create_vertex_array(m_dummyArray);
			assert(stat);

			stat = res::try_create_vertex_array(m_vertexArray);
			assert(stat);

			glEnableVertexArrayAttrib(m_vertexArray.id, 0);
			glVertexArrayVertexBuffer(m_vertexArray.id, 0, m_gfxPoints->id(), 0, sizeof(GfxBuffer2::vec));
			glVertexArrayBindingDivisor(m_vertexArray.id, 0, 0);
			glVertexArrayAttribFormat(m_vertexArray.id, 0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(m_vertexArray.id, 0, 0);
			
			glEnableVertexArrayAttrib(m_vertexArray.id, 1);
			glVertexArrayVertexBuffer(m_vertexArray.id, 1, m_gfxColors->id(), 0, sizeof(GfxBuffer4::vec));
			glVertexArrayBindingDivisor(m_vertexArray.id, 1, 0);
			glVertexArrayAttribFormat(m_vertexArray.id, 1, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(m_vertexArray.id, 1, 1);
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
			// TODO
			m_points.reserve(max_points);
		}

		void deinitPoints()
		{
			// TODO
			m_points.clear();
		}

		void initQuadTree()
		{
			// TODO
		}

		void deinitQuadTree()
		{
			// TODO
		}

	private: // gui callbacks
		void generatePoints(u32 amount)
		{
			m_pointsGenerated = amount;

			auto [w, h] = m_window->framebufferSize();

			auto seed = std::random_device()();
			std::minstd_rand0 base(seed);
			std::uniform_real_distribution<prim::Float> genX(0.05 * w, 0.95 * w);
			std::uniform_real_distribution<prim::Float> genY(0.05 * h, 0.95 * h);

			m_points.clear();
			for (u32 i = 0; i < amount; i++)
				m_points.push_back(prim::Vec2{genX(base), genY(base)});

			for (u32 i = 0; i < amount; i++)
				m_gfxPoints->write(m_points[i], i);
			m_gfxPoints->insertSync();

			for (u32 i = 0; i < amount; i++)
				m_gfxColors->write(m_color0, i);
			m_gfxColors->insertSync();

			std::cout << "[LAB1] seed: " << seed << std::endl;
		}

		void returnBack()
		{
			m_goBack = true;
		}

		void frameParamsChanged(f32 x0, f32 x1, f32 y0, f32 y1)
		{
			m_frameParams[0] = x0;
			m_frameParams[1] = x1;
			m_frameParams[2] = y0;
			m_frameParams[3] = y1;
			
			// TODO
			//doQuery(m_frameParams);
		}

	private: // input callbacks
		void mouseButton(int button, int action, int mods)
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

		void mouseMoved(double xpos, double ypos)
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

				// TODO
				//doQuery(params);
			}
		}

	private: // operations
		bool inFrame(double x, double y)
		{
			return m_frameParams[0] <= x && x <= m_frameParams[1] && m_frameParams[2] <= y && y <= m_frameParams[3];
		}

		prim::mat4 projMat()
		{
			// projects shitty top-left originated window into OpenGL coordinate system
			auto [w, h] = m_window->framebufferSize();

			prim::mat4 proj{1.0};
			proj[0][0] = +2.0 / w; 
			proj[1][1] = -2.0 / h;
			proj[3][0] = -1.0;
			proj[3][1] = +1.0;
			return proj;
		}

		void doQuery(const prim::vec4& params)
		{
			// TODO : can be dangerous, mapping is coherent and there are writes to different locations 
			// TODO : maybe double buffering will help
			m_gfxColors->waitSync();
			for (auto& handle : m_query)
				m_gfxColors->write(m_color0, handle);

			m_tree->query({{m_frameParams[0], m_frameParams[2]}, {m_frameParams[1], m_frameParams[3]}}, m_query);

			for (auto& handle : m_query)
				m_gfxColors->write(m_color1, handle);
			m_gfxColors->insertSync();
		}

		void renderPoints()
		{
			// TODO : paranoia, check synchronization

			auto [w, h] = m_window->framebufferSize();

			GLint64 bound{};
			glGetInteger64v(GL_DRAW_FRAMEBUFFER_BINDING, &bound);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_renderedPoints.id);
			glViewport(0, 0, w, h);
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_progColorPrim->use();
			m_progColorPrim->setProj(projMat());

			glBindVertexArray(m_vertexArray.id);
			glDrawArrays(GL_POINTS, 0, m_pointsGenerated);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bound);
		}

		void renderAttachment()
		{
			// TODO : paranoia
			glBindVertexArray(m_dummyArray.id);
			m_progRect->use();
			m_progRect->setTexture(m_attachment.id);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		void renderFrame()
		{
			// TODO : paranoia
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
		bool m_paused{false};

		// gui & state
		std::unique_ptr<Lab1Gui> m_gui;
		sig::Connection m_goBackConn;
		sig::Connection m_frameChangedConn;
		sig::Connection m_generatePointsConn;
		prim::vec4 m_frameParams{};
		prim::vec4 m_framePrev{};
		u32        m_pointsGenerated{};
		bool       m_goBack{false};

		// input & state
		sig::Connection m_mouseButtonConn;
		sig::Connection m_mouseMovedConn;
		double m_capturedX{};
		double m_capturedY{};
		bool m_frameCaptured{};

		// gfx
		res::VertexArray m_dummyArray;
		res::VertexArray m_vertexArray;
		res::Framebuffer m_renderedPoints;
		res::Texture     m_attachment;
		std::unique_ptr<GfxBuffer2> m_gfxPoints;
		std::unique_ptr<GfxBuffer4> m_gfxColors;
		std::unique_ptr<ProgColorPrim> m_progColorPrim;
		std::unique_ptr<ProgRect>      m_progRect;
		std::unique_ptr<ProgFrame>     m_progFrame;
		prim::vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		prim::vec4 m_color1{1.0, 0.0, 0.0, 1.0};
		prim::vec4 m_color2{0.0, 0.0, 0.0, 1.0};

		// points
		std::vector<prim::Vec2>   m_points;
		std::vector<Handle>       m_query;
		std::unique_ptr<QuadTree> m_tree;
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
