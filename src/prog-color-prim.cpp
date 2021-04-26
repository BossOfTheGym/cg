#include "prog-color-prim.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>

namespace
{
	const char* VERT_SOURCE = R"vert(
	#version 450 core

	layout(location = 0) in vec2 pos;
	layout(location = 1) in vec4 color;

	out vec4 vert_color;

	uniform mat4 proj;

	void main()
	{
		vert_color = color;
		gl_Position = proj * vec4(pos, 0.0, 1.0);
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
	bool stat{};

	res::Shader vert;
	stat = res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE);
	assert(stat);

	res::Shader frag;
	stat = res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE);
	assert(stat);

	stat = res::try_create_shader_program(m_prog, vert, frag);
	assert(stat);

	m_projLoc = glGetUniformLocation(m_prog.id, "proj");
	assert(m_projLoc != -1);
}

void ProgColorPrim::setProj(const prim::mat4& proj)
{
	glProgramUniformMatrix4fv(m_prog.id, m_projLoc, 1, GL_FALSE, prim::value_ptr(proj));
}

void ProgColorPrim::use()
{
	glUseProgram(m_prog.id);
}
