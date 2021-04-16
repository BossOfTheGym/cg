#include "graphics-res-util.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

namespace
{
	std::map<std::string, GLenum> EXT_TO_SHADER_TYPE = 
	{
		{".vert", GL_VERTEX_SHADER}, 
		{".tesc", GL_TESS_CONTROL_SHADER},
		{".tese", GL_TESS_EVALUATION_SHADER},
		{".geom", GL_GEOMETRY_SHADER},
		{".frag", GL_FRAGMENT_SHADER},
		{".comp", GL_COMPUTE_SHADER}
	};
}

namespace res
{
	// shaders
	GLenum shader_type_from_extension(const fs::path& file)
	{
		auto ext = file.extension().string();
		if (auto it = EXT_TO_SHADER_TYPE.find(ext); it != EXT_TO_SHADER_TYPE.end())
		{
			return it->second;
		}
		/*if (ext == ".vert")
		{
			return GL_VERTEX_SHADER;	
		}
		if (ext == ".tesc")
		{
			return GL_TESS_CONTROL_SHADER;
		}
		if (ext == ".tese")
		{
			return GL_TESS_EVALUATION_SHADER;
		}
		if (ext == ".geom")
		{
			return GL_GEOMETRY_SHADER;
		}
		if (ext == ".frag")
		{
			return GL_FRAGMENT_SHADER;
		}
		if (ext == ".comp")
		{
			return GL_COMPUTE_SHADER;
		}*/

		return -1;
	}

	std::string get_shader_info_log(const Shader& shader)
	{
		GLint length{};
		glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &length);

		std::string infoLog; 
		infoLog.resize(length);
		glGetShaderInfoLog(shader.id, length, nullptr, infoLog.data());

		return infoLog;
	}

	Shader create_shader_from_source(GLenum shaderType, const std::string& source)
	{
		Shader shader{};

		shader.id = glCreateShader(shaderType);

		const char* src = source.c_str();
		GLsizei size = source.size();
		glShaderSource(shader.id, 1, &src, &size);

		glCompileShader(shader.id);

		GLint compileStatus{};
		glGetShaderiv(shader.id, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			std::cerr << "Failed to compile shader. Error log: " << std::endl;
			std::cerr << get_shader_info_log(shader) << std::endl;

			shader.reset();
		}
		return shader;
	}

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path)
	{
		auto filePath = path.string();

		std::ifstream input(filePath);
		if (!input.is_open())
		{
			std::cerr << "Failed to open file " << std::quoted(filePath) << std::endl;

			return Shader{};
		}

		auto start  = std::istreambuf_iterator<char>(input);
		auto finish = std::istreambuf_iterator<char>();
		std::string contents(start, finish);

		return create_shader_from_source(shaderType, contents);
	}

	bool try_create_shader_from_source(Shader& shader, GLenum shaderType, const std::string& source)
	{
		shader = create_shader_from_source(shaderType, source);

		return shader.valid();
	}

	bool try_create_shader_from_file(Shader& shader, GLenum shaderType, const fs::path& file)
	{
		shader = create_shader_from_file(shaderType, file);

		return shader.valid();
	}


	// shader program
	std::string get_shader_program_info_log(const ShaderProgram& program)
	{
		GLint length{};
		glGetProgramiv(program.id, GL_INFO_LOG_LENGTH, &length);

		std::string infoLog;
		infoLog.resize(length);
		glGetProgramInfoLog(program.id, length, nullptr, infoLog.data());

		return infoLog;
	}

	ShaderProgram create_shader_program(Shader** shaders, i32 count)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = glCreateProgram();
		for (i32 i = 0; i < count; i++)
		{
			glAttachShader(shaderProgram.id, shaders[i]->id);
		}

		glLinkProgram(shaderProgram.id);
		for (i32 i = 0; i < count; i++)
		{
			glDetachShader(shaderProgram.id, shaders[i]->id);
		}

		GLint linkStatus{};
		glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			std::cerr << "Failed to link shader program. Error log: " << std::endl;
			std::cerr << get_shader_program_info_log(shaderProgram) << std::endl;

			shaderProgram.reset();
		}

		return shaderProgram;
	}

	bool try_create_shader_program(ShaderProgram& program, Shader** shaders, i32 count)
	{
		program = create_shader_program(shaders, count);

		return program.valid();
	}


	// textures
	Texture create_texture(i32 width, i32 height, GLenum format)
	{
		Texture texture{};

		glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
		if (!texture.valid())
		{
			return Texture{};
		}

		glTextureStorage2D(texture.id, 1, format, width, height);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return texture;
	}

	Texture create_test_texture(i32 width, i32 height, i32 period)
	{
		Texture texture = create_texture(width, height, GL_RGBA32F);
		if (!texture.valid())
		{
			return Texture{};
		}

		std::vector<f32> data(width * height * 4);

		const f32 PI = 3.14159265359;
		const f32 K = 2 * PI / period;

		auto ptr = data.begin();
		for (i32 i = 0; i < height; i++)
		{
			for (i32 j = 0; j < width; j++)
			{
				f32 c = std::abs(std::sin(K * j) + std::sin(K * i));

				*ptr++ = c;
				*ptr++ = c;
				*ptr++ = c;
				*ptr++ = 1.0;
			}
		}

		glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data.data());

		return texture;
	}

	bool try_create_texture(Texture& texture, i32 width, i32 height, GLenum format)
	{
		texture = create_texture(width, height, format);

		return texture.valid();
	}

	bool try_create_test_texture(Texture& texture, i32 width, i32 height, i32 period)
	{
		texture = create_test_texture(width, height, period);

		return texture.valid();
	}


	// vertex array
	VertexArray create_vertex_array()
	{
		VertexArray array{};

		glCreateVertexArrays(1, &array.id);

		return array;
	}

	bool try_create_vertex_array(VertexArray& vertexArray)
	{
		vertexArray = create_vertex_array();

		return vertexArray.valid();
	}


	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags, void* data)
	{
		Buffer buffer{};

		glCreateBuffers(1, &buffer.id);
		if (!buffer.valid())
		{
			return Buffer{};
		}

		glNamedBufferStorage(buffer.id, size, data, usageFlags);

		return buffer;
	}

	bool try_create_storage_buffer(Buffer& buffer, GLsizeiptr size, GLbitfield usageFlags, void* data)
	{
		buffer = create_storage_buffer(size, usageFlags, data);

		return buffer.valid();
	}


	// query
	Query create_query()
	{
		Query query{};

		glGenQueries(1, &query.id);

		return query;
	}

	bool try_create_query(Query& query)
	{
		query = create_query();

		return query.valid();
	}


	// fence
	FenceSync create_fence_sync()
	{
		FenceSync sync{};

		// GL_SYNC_GPU_COMMANDS_COMPLETE is the only available option for now
		// flags must be zero for now
		sync.id = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		return sync;
	}

	bool try_create_sync(FenceSync& sync)
	{
		sync = create_fence_sync();

		return sync.valid();
	}


	// buffer range
	BufferRange create_buffer_range(const Buffer& buffer, GLintptr offset, GLsizei size)
	{
		BufferRange range{};
		range.bufferId = buffer.id;
		range.offset   = offset;
		range.size     = size;
		return range;
	}
}