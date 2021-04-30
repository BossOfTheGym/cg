#include "lab2.h"
#include "lab2-gui.h"

#include "app.h"
#include "section.h"
#include "primitive.h"
#include "prog-rect.h"
#include "main-window.h"
#include "prog-color-prim.h"
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

namespace
{
	constexpr const u32 max_segments = 1'00'000;
	constexpr const Float pi = 3.14159265358979323846;

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

		vec* ptr()
		{
			return m_mappedPtr;
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


		vec* frontPtr()
		{
			return Base::ptr() + m_frontOffset;
		}

		vec* backPtr()
		{
			return Base::ptr() + m_backOffset;
		}


		void swap()
		{
			m_front ^= 0x1u;

			std::swap(m_frontOffset, m_backOffset);
		}

		void resize(u32 value)
		{
			assert(value <= Base::capacity() / 2);

			waitSync();

			m_size = value;
			m_front = 0u;
			m_frontOffset = 0u;
			m_backOffset = m_size;
		}

		u32 size() const
		{
			return m_size;
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

	using GfxDBuffer2 = GfxDBuffer<vec2>;
	using GfxDBuffer3 = GfxDBuffer<vec3>;
	using GfxDBuffer4 = GfxDBuffer<vec4>;

	using Handle = u32;

	class Lab2Impl
	{
	public:
		Lab2Impl(Lab2* lab) : m_lab(lab), m_window(lab->app().window())
		{}

		~Lab2Impl()
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
			initSegments();
			initGen();
			initState();

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			deinitState();
			deinitGen();
			deinitSegments();
			deinitGfx();
			deinitGui();

			m_initialized = false;
		}

		AppAction execute()
		{
			doGui();
			render();
			swapDBuffered();
			return handleState();
		}

	private: // init & deinit functions
		void initGui()
		{
			m_gui.reset(new Lab2Gui(max_segments, 0u));
			assert(m_gui != nullptr);

			m_intersectConn = m_gui->intersect.connect([&](){ onIntersect(); });;
			m_generateConn  = m_gui->generate .connect([&](u32 count){ onGenerate(count); });
			m_clearConn     = m_gui->clear    .connect([&](){ onClear(); });
			m_backConn      = m_gui->back     .connect([&](){ onBack(); });
		}

		void deinitGui()
		{
			m_backConn.release();
			m_clearConn.release();
			m_generateConn.release();
			m_intersectConn.release();
			m_gui.reset();
		}

		void initGfx()
		{
			auto [w, h] = m_window->framebufferSize();

			bool stat{};
			
			m_gfxVertices.reset(new GfxDBuffer2(2 * max_segments));
			assert(m_gfxVertices != nullptr);
			m_gfxColors.reset(new GfxDBuffer4(2 * max_segments));
			assert(m_gfxColors != nullptr);

			m_progColorPrim.reset(new ProgColorPrim());
			assert(m_progColorPrim != nullptr);
			m_progRect.reset(new ProgRect());
			assert(m_progRect != nullptr);

			stat = res::try_create_texture(m_attachment);
			assert(stat);
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA8, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stat = res::try_create_framebuffer(m_renderedSegments); 
			assert(stat);
			glNamedFramebufferTexture(m_renderedSegments.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_renderedSegments.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_renderedSegments.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			stat = res::try_create_vertex_array(m_dummyArray);
			assert(stat);

			m_vertexArray.reset(new DVertexArray(0u));
			assert(m_vertexArray != nullptr);
			auto arrayId = m_vertexArray->id();
			glEnableVertexArrayAttrib(arrayId, 0);
			glVertexArrayVertexBuffer(arrayId, 0, m_gfxVertices->id(), 0, sizeof(GfxDBuffer2::vec));
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

			m_renderedSegments.reset();
			m_attachment.reset();

			m_progRect.reset();
			m_progColorPrim.reset();

			m_gfxColors.reset();
			m_gfxVertices.reset();

		}

		void initSegments()
		{
			m_segments.reserve(max_segments);
			m_segmentHandles.reserve(max_segments);
		}

		void deinitSegments()
		{
			m_segments.clear();
			m_segmentHandles.clear();
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

		void initGen()
		{
			m_seed.seed(1); // std::random_device()()
		}

		void deinitGen()
		{}

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

				generateSegments();
			}

			if (m_needIntersect)
			{
				m_needIntersect = false;
				
				intersect();
			}

			if (m_needClear)
			{
				m_needClear = false;

				clear();
			}


			return {};
		}


	private: // gui & gui callbacks
		void doGui()
		{
			m_gui->draw();
		}

		void onGenerate(u32 count)
		{
			m_segmentsToGen = count;

			m_needGenerate = true;
		}

		void onIntersect()
		{
			m_needIntersect = true;
		}

		void onClear()
		{
			m_needClear = true;
		}

		void onBack()
		{
			m_needGoBack = true;
		}


	private: // utilities
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
			m_needIntersect = false;
			m_needGenerate = false;
			m_needClear = false;
			m_needGoBack = false;
			m_needRedraw = false;
			m_needSwap = false;
		}


	private: // operations
		void generateSegments()
		{
			auto [w, h] = m_window->framebufferSize();

			// segments
			auto seed = m_seed();
			std::cout << "seed: " << seed << std::endl;

			std::minstd_rand0 base(seed);
			std::uniform_real_distribution<Float> genX(0.1 * w, 0.9 * w); // x
			std::uniform_real_distribution<Float> genY(0.1 * h, 0.9 * h); // y
			std::uniform_real_distribution<Float> genA(0.0, 2 * pi); // angle
			std::uniform_real_distribution<Float> genD(0.05 * w, 0.1 * w);  // dist

			m_segments.clear();
			for (u32 i = 0; i < m_segmentsToGen; i++)
			{
				auto x0 = genX(base);
				auto y0 = genY(base);
				auto a = genA(base);
				auto d = genD(base);
				auto x1 = d * std::cos(a) + x0;
				auto y1 = d * std::sin(a) + y0;
				
				m_segments.push_back({{x0, y0}, {x1, y1}});
			}
			
			m_segmentHandles.clear();
			for (u32 i = 0; i < m_segmentsToGen; i++)
				m_segmentHandles.push_back(i);

			// vertex array
			m_vertexArray->primitives(2 * m_segmentsToGen);

			// vertices
			m_gfxVertices->resize(2 * m_segmentsToGen);
			vec2* vertPtr = m_gfxVertices->frontPtr();
			for (u32 i = 0; i < m_segmentsToGen; i++)
			{
				vertPtr[2 * i    ] = m_segments[i].v0;
				vertPtr[2 * i + 1] = m_segments[i].v1;
			}
			vertPtr = m_gfxVertices->backPtr();
			for (u32 i = 0; i < m_segmentsToGen; i++)
			{
				vertPtr[2 * i    ] = m_segments[i].v0;
				vertPtr[2 * i + 1] = m_segments[i].v1;
			}
			m_gfxVertices->flush();
			m_gfxVertices->sync();

			// colors
			m_gfxColors->resize(2 * m_segmentsToGen);
			vec4* colPtr = m_gfxColors->frontPtr();
			for (u32 i = 0 ; i < 2 * m_segmentsToGen; i++)
				colPtr[i] = m_color0;
			colPtr = m_gfxColors->backPtr();
			for (u32 i = 0 ; i < 2 * m_segmentsToGen; i++)
				colPtr[i] = m_color1;
			m_gfxColors->flush();
			m_gfxColors->sync();

			m_needRedraw = true;
		}

		void intersect()
		{
			auto sampler = [&] (Handle handle)
			{
				assert(handle < m_segments.size());

				return m_segments[handle];
			};

			auto intersections = sect::section_n_lines(m_segmentHandles, sampler);
			std::cout << "intersections:" << std::endl;
			for (auto& [point, lines] : intersections)
				std::cout << point.x << " " << point.y << std::endl;

			m_gfxColors->waitSyncBack();

			auto colorPtr = m_gfxColors->backPtr();
			for (u32 i = 0; i < m_gfxColors->size(); i++)
				colorPtr[i] = m_color0;
			for (auto& [point, lines] : intersections)
				for (auto& handle : lines)
					colorPtr[handle] = m_color1;

			m_gfxColors->flushBack();
			m_gfxColors->syncBack();

			m_gui->setIntersectionInfo(0u);

			m_needSwap   = true;
			m_needRedraw = true;
		}

		void clear()
		{
			m_segments.clear();
			m_segmentHandles.clear();

			m_vertexArray->primitives(0u);

			m_gfxVertices->resize(0u);
			m_gfxColors->resize(0u);

			m_needRedraw = true;
		}


	private: // double-buffer control
		void swapDBuffered()
		{
			if (m_needSwap)
			{
				m_needSwap = false;

				m_gfxColors->swap();
				m_gfxVertices->swap();
				m_vertexArray->swap();
			}
		}


	private: // rendering
		void render()
		{
			renderSegments();
			renderAttachment();
		}

		void renderSegments()
		{	
			if (!m_needRedraw)
				return;
			m_needRedraw = false;

			m_gfxColors->waitSyncFront();

			auto [w, h] = m_window->framebufferSize();

			GLint64 bound{};
			glGetInteger64v(GL_DRAW_FRAMEBUFFER_BINDING, &bound);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_renderedSegments.id);
			glViewport(0, 0, w, h);
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_progColorPrim->use();
			m_progColorPrim->setProj(projMat());

			glBindVertexArray(m_vertexArray->id());
			glDrawArrays(GL_LINES, m_vertexArray->front(), m_vertexArray->primitives());

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bound);
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
		Lab2* m_lab{};
		MainWindow* m_window{};
		bool m_initialized{false};

		// gui
		std::unique_ptr<Lab2Gui> m_gui;
		sig::Connection m_generateConn;
		sig::Connection m_intersectConn;
		sig::Connection m_clearConn;
		sig::Connection m_backConn;

		// gfx
		res::VertexArray m_dummyArray;
		res::Framebuffer m_renderedSegments;
		res::Texture     m_attachment;
		std::unique_ptr<ProgColorPrim> m_progColorPrim;
		std::unique_ptr<ProgRect>      m_progRect;
		std::unique_ptr<DVertexArray> m_vertexArray;
		std::unique_ptr<GfxDBuffer2>  m_gfxVertices;
		std::unique_ptr<GfxDBuffer4>  m_gfxColors;
		vec4 m_color0{0.0, 0.0, 0.0, 1.0};
		vec4 m_color1{1.0, 0.0, 0.0, 1.0};

		// state
		std::minstd_rand m_seed;
		u32 m_segmentsToGen{};

		bool m_needIntersect{false};
		bool m_needGenerate{false};
		bool m_needGoBack{false};
		bool m_needRedraw{false};
		bool m_needClear{false};
		bool m_needSwap{false};

		// segments
		std::vector<Line2>  m_segments{};
		std::vector<Handle> m_segmentHandles{};
	};
}


Lab2::Lab2(App* app) : AppState(app)
{
	m_impl.reset(new Lab2Impl(this));
	assert(m_impl != nullptr);
}

Lab2::~Lab2()
{
	deinit();

	m_impl.reset();
}

bool Lab2::init()
{
	return m_impl->init();
}

void Lab2::deinit()
{
	m_impl->deinit();
}

AppAction Lab2::execute()
{
	return m_impl->execute();
}
