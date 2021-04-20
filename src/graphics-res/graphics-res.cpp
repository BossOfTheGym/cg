#include "graphics-res.h"

#include <utility>

namespace res
{
	// Shader
	Shader::Shader(Shader&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	Shader::~Shader()
	{
		reset();
	}

	Shader& Shader::operator = (Shader&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void Shader::reset()
	{
		if (id != null)
		{
			glDeleteShader(id);

			id = null;
		}
	}

	bool Shader::valid() const
	{
		return id != null;
	}


	// ShaderProgram
	ShaderProgram::ShaderProgram(ShaderProgram&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	ShaderProgram::~ShaderProgram()
	{
		reset();
	}

	ShaderProgram& ShaderProgram::operator = (ShaderProgram&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void ShaderProgram::reset()
	{
		if (id != null)
		{
			glDeleteProgram(id);

			id = null;
		}
	}

	bool ShaderProgram::valid() const
	{
		return id != null;
	}


	// Buffer
	Buffer::Buffer(Buffer&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	Buffer::~Buffer()
	{
		reset();
	}

	Buffer& Buffer::operator = (Buffer&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void Buffer::reset()
	{
		if (id != null)
		{
			glDeleteBuffers(1, &id);

			id = null;
		}	
	}

	bool Buffer::valid() const
	{
		return id != null;
	}


	// VertexArray
	VertexArray::VertexArray(VertexArray&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	VertexArray::~VertexArray()
	{
		reset();
	}

	VertexArray& VertexArray::operator = (VertexArray&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void VertexArray::reset()
	{
		if (id != null)
		{
			glDeleteVertexArrays(1, &id);

			id = null;
		}
	}

	bool VertexArray::valid() const
	{
		return id != null;
	}


	// Texture
	Texture::Texture(Texture&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	Texture::~Texture()
	{
		reset();
	}

	Texture& Texture::operator = (Texture&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void Texture::reset()
	{
		if (id != null)
		{
			glDeleteTextures(1, &id);

			id = null;
		}
	}

	bool Texture::valid() const
	{
		return id != null;
	}


	// Query
	Query::Query(Query&& another) noexcept : id{std::exchange(another.id, null)}
	{}

	Query::~Query()
	{
		reset();
	}

	Query& Query::operator = (Query&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		id = std::exchange(another.id, null);

		return *this;
	}

	void Query::reset()
	{
		if (id != null)
		{
			glDeleteQueries(1, &id);

			id = null;
		}
	}

	bool Query::valid() const
	{
		return id != null;
	}

	// Framebuffer
	Framebuffer::Framebuffer(Framebuffer&& another) noexcept
		: id(std::exchange(another.id, null))
	{}

	Framebuffer::~Framebuffer()
	{
		reset();
	}

	Framebuffer& Framebuffer::operator = (Framebuffer&& another) noexcept
	{
		if (this != &another)
		{
			reset();

			id = std::exchange(another.id, null);
		}
		return *this;
	}

	void Framebuffer::reset()
	{
		if (valid())
		{
			glDeleteFramebuffers(1, &id);

			id = null;
		}
	}

	bool Framebuffer::valid() const
	{
		return id != null;
	}


	// FenceSync
	FenceSync::FenceSync(FenceSync&& another) noexcept : id{std::exchange(another.id, nullptr)}
	{}

	FenceSync::~FenceSync()
	{
		del();
	}

	FenceSync& FenceSync::operator = (FenceSync&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		del();

		id = std::exchange(another.id, nullptr);

		return *this;
	}

	void FenceSync::del()
	{
		if (id != nullptr)
		{
			glDeleteSync(id);
			
			id = nullptr;
		}
	}

	bool FenceSync::valid() const
	{
		return id != nullptr;
	}



	// BufferRange
	BufferRange::BufferRange(BufferRange&& another) noexcept
		: bufferId{std::exchange(another.bufferId, null)}
		, offset{std::exchange(another.offset, 0)}
		, size(std::exchange(another.size, 0))
	{}

	BufferRange::~BufferRange()
	{
		reset();
	}

	BufferRange& BufferRange::operator = (BufferRange&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		bufferId = std::exchange(another.bufferId, null);
		offset = std::exchange(another.offset, 0);
		size = std::exchange(another.size, 0);

		return *this;
	}

	void BufferRange::reset()
	{
		if (bufferId != null)
		{
			bufferId = null;
			offset = 0;
			size = 0;
		}
	}

	bool BufferRange::valid() const
	{
		return bufferId != null;
	}


	// MapPointer
	MapPointer::MapPointer(MapPointer&& another) noexcept
		: bufferId{std::exchange(another.bufferId, null)}
		, offset{std::exchange(another.offset, 0)}
		, size{std::exchange(another.size, 0)}
		, ptr{std::exchange(another.ptr, nullptr)}
	{}

	MapPointer::~MapPointer()
	{
		reset();
	}

	MapPointer& MapPointer::operator = (MapPointer&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		reset();

		bufferId = std::exchange(another.bufferId, null);
		offset = std::exchange(another.offset, 0);
		size = std::exchange(another.size, 0);
		ptr = std::exchange(another.ptr, nullptr);

		return *this;
	}

	void MapPointer::reset()
	{
		if (bufferId != null)
		{
			glUnmapNamedBuffer(bufferId);

			bufferId = null;
			offset = 0;
			size = 0;
			ptr = nullptr;
		}
	}

	bool MapPointer::valid() const
	{
		return bufferId != null;
	}
}