#pragma once

#include "../gl-header/gl-header.h"

namespace res
{
	// TODO : refactor
	// TODO : create CRTP template
	// TODO : BufferRAnge can be moved to graphics-res-util and used as util(no no-copy restrictiion)
	
	// all resources are just guards, their only purpose is to maintain scoped lifetime
	// header "graphics-res-util.h" contains utility functions that create some resources

	using Id = GLuint;

	constexpr const Id null = 0u; 
	constexpr const Id default_framebuffer = null;


	struct Shader
	{		
		Shader() = default;

		Shader(Shader&& another) noexcept;
		Shader(const Shader&) = delete;

		~Shader();

		Shader& operator = (Shader&& another) noexcept;
		Shader& operator = (const Shader&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct ShaderProgram
	{
		ShaderProgram() = default;

		ShaderProgram(ShaderProgram&& another) noexcept;
		ShaderProgram(const ShaderProgram&) = delete;

		~ShaderProgram();

		ShaderProgram& operator = (ShaderProgram&& another) noexcept;
		ShaderProgram& operator = (const ShaderProgram&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Buffer
	{
		Buffer() = default;

		Buffer(Buffer&& another) noexcept;
		Buffer(const Buffer&) = delete;

		~Buffer();

		Buffer& operator = (Buffer&& another) noexcept;
		Buffer& operator = (const Buffer&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct VertexArray
	{
		VertexArray() = default;

		VertexArray(VertexArray&& another) noexcept;
		VertexArray(const VertexArray&) = delete;

		~VertexArray();

		VertexArray& operator = (VertexArray&& another) noexcept;
		VertexArray& operator = (const VertexArray&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Texture
	{
		Texture() = default;

		Texture(Texture&& another) noexcept;
		Texture(const Texture&) = delete;

		~Texture();

		Texture& operator = (Texture&& another) noexcept;
		Texture& operator = (const Texture&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};

	struct Query
	{
		Query() = default;

		Query(Query&& another) noexcept;
		Query(const Query&) = delete;

		~Query();

		Query& operator = (Query&& another) noexcept;
		Query& operator = (const Query&) = delete;


		void reset();

		bool valid() const;


		Id id{null};
	};


	struct FenceSync
	{
		FenceSync() = default;

		FenceSync(FenceSync&& another) noexcept;
		FenceSync(const FenceSync&) = delete;

		~FenceSync();

		FenceSync& operator = (FenceSync&& another) noexcept;
		FenceSync& operator = (const FenceSync&) = delete;


		void del();

		bool valid() const;


		GLsync id{nullptr};
	};


	struct BufferRange
	{
		BufferRange() = default;

		BufferRange(BufferRange&& another) noexcept;
		BufferRange(const BufferRange&) = delete;

		~BufferRange();

		BufferRange& operator = (BufferRange&& another) noexcept;
		BufferRange& operator = (const BufferRange&) = delete;


		void reset();

		bool valid() const;


		Id bufferId{null};

		GLintptr offset{};
		GLsizeiptr size{};
	};

	struct MapPointer
	{
		MapPointer() = default;
		
		MapPointer(MapPointer&& another) noexcept;
		MapPointer(const MapPointer&) = delete;

		~MapPointer();

		MapPointer& operator = (MapPointer&& another) noexcept;
		MapPointer& operator = (const MapPointer&) = delete;


		void reset();

		bool valid() const;


		Id bufferId{null};

		GLintptr offset{};
		GLsizeiptr size{};

		void* ptr{nullptr};
	};
}
