#pragma once

#include "../core.h"
#include "graphics-res.h"

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <type_traits>


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
	namespace detail
	{
		// proxy functions to evade inclusion if heavy header required by templates
		GLuint glCreateProgram_proxy();
		void glAttachShader_proxy(GLuint program, GLuint shader);
		void glLinkProgram_proxy(GLuint program);
		void glDetachShader_proxy(GLuint program, GLuint shader);
	}

	std::string get_shader_program_info_log(const ShaderProgram& program);

	bool check_program_link_status(const ShaderProgram& program);

	template<class ... shader_t, std::enable_if_t<(std::is_same_v<std::remove_reference_t<std::remove_cv_t<shader_t>>, Shader> && ...), int> = 0>
	ShaderProgram create_shader_program(shader_t&& ... shader)
	{
		ShaderProgram shaderProgram{};

		shaderProgram.id = detail::glCreateProgram_proxy();
		(detail::glAttachShader_proxy(shaderProgram.id, shader.id), ...);
		detail::glLinkProgram_proxy(shaderProgram.id);
		(detail::glDetachShader_proxy(shaderProgram.id, shader.id), ...);

		if (!check_program_link_status(shaderProgram))
		{
			std::cerr << "Failed to link shader program. Error log: " << std::endl;
			std::cerr << get_shader_program_info_log(shaderProgram) << std::endl;

			shaderProgram.reset();
		}

		return shaderProgram;
	}

	ShaderProgram create_shader_program(Shader** shaders, i32 count);

	template<class ... shader_t>
	auto try_create_shader_program(ShaderProgram& program, shader_t&& ... shader) 
		-> std::enable_if_t<(std::is_same_v<std::remove_reference_t<std::remove_cv_t<shader_t>>, Shader> && ...), bool>
		// disambiguation with the function going after
	{
		program = create_shader_program(std::forward<Shader>(shader)...);

		return program.valid();
	}

	bool try_create_shader_program(ShaderProgram& program, Shader** shaders, i32 count);


	// textures
	Texture create_texture(i32 width, i32 height, GLenum format);

	Texture create_test_texture(i32 width, i32 height, i32 period);

	Texture create_stencil_texture(i32 width, i32 height);


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


	// framebuffer
	Framebuffer create_framebuffer();

	bool try_create_framebuffer(Framebuffer& framebuffer);


	// fence
	FenceSync create_fence_sync();

	bool try_create_sync(FenceSync& sync);


	// buffer range
	BufferRange create_buffer_range(const Buffer& buffer, GLintptr offset, GLsizei size);
}
