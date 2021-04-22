#include "prog-color-prim.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>

namespace
{
	const char* VERT_SOURCE = R"vert(
	#version 450 core

	in vec2 pos;
	in vec4 color;

	out vec4 vert_color;

	void main()
	{
		vert_color = color;
		gl_Position = vec4(pos, 0.0, 1.0);
	}
	)vert";

	const char* FRAG_SOURCE = R"frag(
	#version 450 core

	out vec4 frag_color;
	in vec4 vert_color;

	void main()
	{
		frag_color = vert_color;
	}
	)frag";
}

ProgColorPrim::ProgColorPrim()
{
	res::Shader vert;
	assert(res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE));

	res::Shader frag;
	assert(res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE));

	assert(res::try_create_shader_program(m_prog, vert, frag));
}

void ProgColorPrim::use()
{
	glUseProgram(m_prog.id);
}
