#pragma once

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