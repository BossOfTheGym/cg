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

	template<class vec_t> // vec2, vec3, vec4
	class GfxBuffer
	{
	public:
		using vec = vec_t;

	public:
		GfxBuffer(u32 initCapacity) : m_capacity(initCapacity)
		{
			bool stat{};

			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
			stat = res::try_create_storage_buffer(m_buffer, initCapacity * sizeof(vec), flags, nullptr);
			assert(stat);

			flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
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

		void flush(u32 start, u32 end)
		{
			assert(start <= m_capacity);
			assert(end <= m_capacity);
			assert(start <= end);

			if (start == end)
				return;

			u32 len = end - start;
			glFlushMappedNamedBufferRange(m_buffer.id, start * sizeof(vec), len * sizeof(vec));
		}

		void flush()
		{
			flush(0, m_capacity);
		}

		void write(const vec& v, u32 index)
		{
			assert(index < m_capacity);

			m_mappedPtr[index] = v;
		}

	private:
		res::Buffer m_buffer{};

		u32 m_capacity{};
		vec* m_mappedPtr{nullptr};
	};

	template<class vec_t> // vec2, vec3, vec4
	class GfxDBuffer : public GfxBuffer<vec_t>
	{
	public:
		using Base = GfxBuffer<vec_t>;
		using vec  = vec_t;

	public:
		GfxDBuffer(u32 initCapacity, u32 wait = 50u) 
			: Base(2 * initCapacity)
			, m_wait(wait)
			, m_size(initCapacity)
		{}

	public:
		void waitSyncFront()
		{
			waitSync(m_syncs[m_front]);
		}

		void waitSyncBack()
		{
			waitSync(m_syncs[m_front ^ 0x1u]);
		}

		void waitSync()
		{
			waitSync(m_syncs[0]);
			waitSync(m_syncs[1]);
		}


		void syncFront()
		{
			createSync(m_syncs[m_front]);
		}

		void syncBack()
		{
			createSync(m_syncs[m_front ^ 0x1u]);
		}

		void sync()
		{
			createSync(m_syncs[0]);
			createSync(m_syncs[1]);
		}

		void writeFront(const vec& v, u32 index)
		{
			Base::write(v, m_frontOffset + index);
		}

		void writeBack(const vec& v, u32 index)
		{
			Base::write(v, m_backOffset + index);
		}


		void flushFront(u32 start, u32 end)
		{
			Base::flush(m_frontOffset + start, m_frontOffset + end);
		}

		void flushFront()
		{
			flushFront(0u, m_size);
		}

		void flushBack(u32 start, u32 end)
		{
			Base::flush(m_backOffset + start, m_backOffset + end);
		}

		void flushBack()
		{
			flushBack(0u, m_size);
		}


		void swap()
		{
			m_front ^= 0x1u;

			std::swap(m_frontOffset, m_backOffset);
		}

		void resize(u32 size)
		{
			assert(size <= Base::capacity() / 2);

			waitSync();

			m_size = size;
			m_front = 0u;
			m_frontOffset = 0u;
			m_backOffset = m_size;
		}

	private:
		void waitSync(res::FenceSync& sync)
		{
			if (sync.valid())
			{
				while (true)
				{
					GLenum result = glClientWaitSync(sync.id, GL_SYNC_FLUSH_COMMANDS_BIT, m_wait);
					if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
						break;
				}
				sync.del();
			}
		}

		void createSync(res::FenceSync& sync)
		{
			waitSync(sync);

			sync = res::create_fence_sync();
		}

	private:
		res::FenceSync m_syncs[2]{}; // pending flushes
		u32 m_wait{};
		u32 m_size{};

		u32 m_front{};
		u32 m_frontOffset{};
		u32 m_backOffset{};
	};

	using GfxDBuffer2 = GfxDBuffer<prim::vec2>;
	using GfxDBuffer3 = GfxDBuffer<prim::vec3>;
	using GfxDBuffer4 = GfxDBuffer<prim::vec4>;

	class DVertexArray
	{
	public:
		DVertexArray(u32 initPrimitives) : m_primitives(initPrimitives)
		{
			bool stat = res::try_create_vertex_array(m_array); assert(stat);
		}

	public:
		res::Id id()
		{
			return m_array.id;
		}

		void swap()
		{
			m_front ^= 0x1u;
		}

		u32 front() const
		{
			return m_front * m_primitives;
		}

		u32 back() const
		{
			return (m_front ^ 0x1u) * m_primitives;
		}


		u32 primitives() const
		{
			return m_primitives;
		}

		void primitives(u32 value)
		{
			m_front = 0;
			m_primitives = value;
		}

	private:
		res::VertexArray m_array;
		u32 m_front{};
		u32 m_primitives{};
	};

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

	// TODO : sequence is broken
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

			m_paused = false;

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

			m_paused = true;

			m_initialized = false;
		}

		AppAction execute()
		{
			swapDBuffered();

			render();
			doGui();

			return act();
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
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA32F, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stat = res::try_create_framebuffer(m_renderedPoints); assert(stat);
			glNamedFramebufferTexture(m_renderedPoints.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_renderedPoints.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_renderedPoints.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			stat = res::try_create_vertex_array(m_dummyArray); assert(stat);

			m_vertexArray.reset(new DVertexArray(0));
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

			m_needSwap = false;
			m_needRedraw = true;
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

			m_needSwap = false;
			m_needRedraw = false;
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


	private:
		AppAction act()
		{
			if (m_goBack)
			{
				m_goBack = false;
				return AppAction{ActionType::Pop}; // we need to pop current state
			}
			return {}; // do nothing
		}

	private: // gui
		void doGui()
		{
			m_gui->draw();
		}

		void generatePoints(u32 amount)
		{
			m_pointsGenerated = amount;

			auto [w, h] = m_window->framebufferSize();

			// points
			auto seed = std::random_device()();
			//seed = 12345; // TODO : for now
			std::minstd_rand0 base(seed);
			std::uniform_real_distribution<prim::Float> genX(0.02 * w, 0.98 * w);
			std::uniform_real_distribution<prim::Float> genY(0.02 * h, 0.98 * h);

			m_points.clear();
			for (u32 i = 0; i < amount; i++)
				m_points.push_back(prim::Vec2{genX(base), genY(base)});

			// vertices
			m_gfxPoints->resize(amount);
			for (u32 i = 0; i < amount; i++)
				m_gfxPoints->writeFront(m_points[i], i);
			for (u32 i = 0; i < amount; i++)
				m_gfxPoints->writeBack(m_points[i], i);
			m_gfxPoints->flush();
			m_gfxPoints->sync();

			// colors
			m_gfxColors->resize(amount);
			for (u32 i = 0; i < amount; i++)
				m_gfxColors->writeFront(m_color0, i);
			for (u32 i = 0; i < amount; i++)
				m_gfxColors->writeBack(m_color0, i);
			m_gfxColors->flush();
			m_gfxColors->sync();

			// vertex array
			m_vertexArray->primitives(m_pointsGenerated);

			// quadtree & query
			m_query->clear();

			auto c = clock();
			m_tree->clear();
			std::cout << "c: " << (float)(clock() - c) / CLOCKS_PER_SEC << "ms" << std::endl;

			c = clock();
			for (u32 i = 0; i < m_pointsGenerated; i++)
				m_tree->insert(i);
			std::cout << "i: " << (float)(clock() - c) / CLOCKS_PER_SEC << "ms" << std::endl;

			doQuery(m_frameParams);
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
			
			doQuery(m_frameParams);
		}


	private: // input
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

				doQuery(m_frameParams);
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

		prim::AABB2 frameToAABB()
		{
			return {{m_frameParams[0], m_frameParams[2]}, {m_frameParams[1], m_frameParams[3]}};
		}

	private: // query
		void doQuery(const prim::vec4& params)
		{
			m_gfxColors->waitSyncBack();

			// reset color
			for (auto& handle : m_query->back())
				m_gfxColors->writeBack(m_color0, handle);

			// query points
			auto c = clock();
			m_tree->query(frameToAABB(), m_query->back());
			std::cout << "q: " << (float)(clock() - c) / CLOCKS_PER_SEC << "ms" << std::endl;

			// set new color
			for (auto& handle : m_query->back())
				m_gfxColors->writeBack(m_color1, handle);

			m_gfxColors->flushBack();
			m_gfxColors->syncBack();

			// data changed so we will need to swap next frame
			m_needSwap = true;
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
				return

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
		bool m_paused{false};

		// gui & state
		std::unique_ptr<Lab1Gui> m_gui;
		sig::Connection m_goBackConn;
		sig::Connection m_frameChangedConn;
		sig::Connection m_generatePointsConn;
		prim::vec4 m_frameParams{};
		u32        m_pointsGenerated{};
		bool       m_goBack{false};

		// input & state
		sig::Connection m_mouseButtonConn;
		sig::Connection m_mouseMovedConn;
		prim::vec4 m_framePrev{};
		double m_capturedX{};
		double m_capturedY{};
		bool m_frameCaptured{};

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
		prim::vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		prim::vec4 m_color1{1.0, 0.0, 0.0, 1.0};
		prim::vec4 m_color2{0.0, 0.0, 1.0, 1.0};
		bool m_needSwap{false};

		// render state
		bool m_needRedraw{false};

		// points
		std::vector<prim::Vec2>         m_points;
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
