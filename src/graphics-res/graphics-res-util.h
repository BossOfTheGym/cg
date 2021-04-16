#pragma once

#include "../core.h"
#include "graphics-res.h"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace res
{
	// TODO : remove error output from functions/ It must be queried separately
	// TODO : return error status

	namespace fs = std::filesystem;
	
	// shaders
	GLenum shader_type_from_extension(const fs::path& file);

	std::string get_shader_info_log(const Shader& shader);

	Shader create_shader_from_source(GLenum shaderType, const std::string& source);

	Shader create_shader_from_file(GLenum shaderType, const fs::path& path);

	bool try_create_shader_from_source(Shader& shader, GLenum shaderType, const std::string& source);

	bool try_create_shader_from_file(Shader& shader, GLenum shaderType, const fs::path& file);


	// shader program
	std::string get_shader_program_info_log(const ShaderProgram& program);

	template<class ... ShaderT>
	ShaderProgram create_shader_program_var(ShaderT&& ... shader)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = glCreateProgram();
		(glAttachShader(shaderProgram.id, shader.id), ...);
		glLinkProgram(shaderProgram.id);
		(glDetachShader(shaderProgram.id, shader.id), ...);

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

	ShaderProgram create_shader_program(Shader** shaders, i32 count);

	template<class ... Shader>
	bool try_create_shader_program_var(ShaderProgram& program, Shader&& ... shader)
	{
		program = create_shader_program(std::forward<Shader>(shader)...);

		return program.valid();
	}

	bool try_create_shader_program(ShaderProgram& program, Shader** shaders, i32 count);


	// textures
	Texture create_texture(i32 width, i32 height, GLenum format);

	Texture create_test_texture(i32 width, i32 height, i32 period);

	bool try_create_texture(Texture& texture, i32 width, i32 height, GLenum format);

	bool try_create_test_texture(Texture& texture, i32 width, i32 height, i32 period);


	// vertex array
	VertexArray create_vertex_array();

	bool try_create_vertex_array(VertexArray& vertexArray);


	// buffer
	Buffer create_storage_buffer(GLsizeiptr size, GLbitfield usageFlags, void* data = nullptr);

	bool try_create_storage_buffer(Buffer& buffer, GLsizeiptr size, GLbitfield usageFlags, void* data = nullptr);


	// query
	Query create_query();

	bool try_create_query(Query& query);


	// fence
	FenceSync create_fence_sync();

	bool try_create_sync(FenceSync& sync);


	// buffer range
	BufferRange create_buffer_range(const Buffer& buffer, GLintptr offset, GLsizei size);
}
