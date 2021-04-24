#include "lab1.h"
#include "lab1-gui.h"

#include "app.h"
#include "quadtree.h"
#include "primitive.h"
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
			GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			assert(res::try_create_storage_buffer(m_buffer, initCapacity * sizeof(vec), flags, nullptr));

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
					GLenum result = glClientWaitSync(m_sync.id, GL_SYNC_FLUSH_COMMANDS_BIT, 10);
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

			return m_mappedPtr[index];
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
	public:
		Lab1Impl(Lab1* lab) 
			: m_lab(lab)
			, m_window(lab->app().window())
		{}

		~Lab1Impl()
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

			m_initialized = true;

			return true;
		}

		void deinit()
		{
			if (!m_initialized)
				return;

			deinitGfx();
			deinitGui();

			m_initialized = false;
		}

		AppAction execute()
		{
			m_gui->draw();
			if (m_goBack)
			{
				m_goBack = false;
				return AppAction{ActionType::Pop};
			}
			return {}; // Nothing
		}

	private: // utility stuff
		auto framebufferSize()
		{
			return m_window->framebufferSize();
		}

	private: // init functions
		void initGui()
		{
			m_gui.reset(new Lab1Gui(max_points, 0u));
			assert(m_gui != nullptr);
			m_generatePointsConn = m_gui->generatePoints    .connect([&](u32 pts){ generatePoints(pts); });
			m_goBackConn         = m_gui->returnBack        .connect([&](){ returnBack(); });
			m_frameChangedConn   = m_gui->frameParamsChanged.connect([&](f32 x0, f32 x1, f32 y0, f32 y1) { frameParamsChanged(x0, x1, y0, y1); });
		}

		void deinitGui()
		{
			m_generatePointsConn.release();
			m_goBackConn.release();
			m_frameChangedConn.release();
			m_gui.reset();
		}

		void initGfx()
		{
			auto [w, h] = framebufferSize();

			m_gfxPoints.reset(new GfxBuffer2(max_points));
			assert(m_gfxPoints != nullptr);
			m_gfxColors.reset(new GfxBuffer4(max_points));
			assert(m_gfxColors != nullptr);

			m_progColorPrim.reset(new ProgColorPrim());
			assert(m_progColorPrim != nullptr);
			m_progFrame.reset(new ProgFrame());
			assert(m_progFrame != nullptr);

			assert(res::try_create_texture(m_attachment));
			glTextureStorage2D(m_attachment.id, 1, GL_RGBA32F, w, h);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_attachment.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			assert(res::try_create_framebuffer(m_renderedPoints));
			glNamedFramebufferTexture(m_renderedPoints.id, GL_COLOR_ATTACHMENT0, m_attachment.id, 0);
			glNamedFramebufferDrawBuffer(m_renderedPoints.id, GL_COLOR_ATTACHMENT0);
			assert(glCheckNamedFramebufferStatus(m_renderedPoints.id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

			assert(res::try_create_vertex_array(m_vertexArray));
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
			m_renderedPoints.reset();
			m_attachment.reset();

			m_progFrame.reset();
			m_progColorPrim.reset();

			m_gfxColors.reset();
			m_gfxPoints.reset();
		}

	private: // callbacks
		void generatePoints(u32 amount)
		{
			// TODO
			std::cout << "generate points: amout" << std::endl;
		}

		void returnBack()
		{
			m_goBack = true;
		}

		void frameParamsChanged(f32 x0, f32 x1, f32 y0, f32 y1)
		{
			// TODO
			std::cout << "frame Changed: " << x0 << " " << x1 << " " << y0 << " " << y1 << std::endl;
		}

	private:
		Lab1*       m_lab{};
		MainWindow* m_window{};
		bool m_initialized{false};

		// gui
		std::unique_ptr<Lab1Gui> m_gui;
		sig::Connection m_goBackConn;
		sig::Connection m_frameChangedConn;
		sig::Connection m_generatePointsConn;
		prim::vec4 m_frameParams{};
		u32        m_pointsGenerated{};
		bool       m_goBack{false};

		// gfx
		res::VertexArray m_vertexArray;
		res::Framebuffer m_renderedPoints;
		res::Texture     m_attachment;
		std::unique_ptr<GfxBuffer2> m_gfxPoints;
		std::unique_ptr<GfxBuffer4> m_gfxColors;
		std::unique_ptr<ProgColorPrim> m_progColorPrim;
		std::unique_ptr<ProgFrame>     m_progFrame;

		// points
		std::vector<prim::Vec2> m_points;

		// TODO
		
		// quadtree
		// quadtree query
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
