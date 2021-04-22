#include "prog-frame.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>


namespace
{
	inline const char* VERT_SOURCE = R"vert(
	#version 450 core

	// x0, x1, y0, y1
	uniform vec4 params;

	// produce:
	// v0 : (x0, y0)
	// v1 : (x0, y1)
	// v2 : (x1, y0)
	// v3 : (x1, y1)
	void main()
	{
		int x = gl_VertexID / 2;
		int y = 2 + gl_VertexID % 2;
		gl_Position = vec4(params[x], params[y], 0.0, 0.5);
	}
	)vert";

	inline const char* FRAG_SOURCE = R"frag(
	#version 450 core

	out vec4 color;

	uniform vec4 frameColor;

	void main()
	{
		color = frameColor;
	}
	)frag";
}

ProgFrame::ProgFrame()
{
	res::Shader vert;
	assert(res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE));

	res::Shader frag;
	assert(res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE));

	assert(res::try_create_shader_program(m_prog, vert, frag));

	m_paramsLoc = glGetUniformLocation(m_prog.id, "params");
	assert(m_paramsLoc != -1);

	m_colorLoc = glGetUniformLocation(m_prog.id, "frameColor");
	assert(m_colorLoc != -1);
}


void ProgFrame::setFrameParams(const prim::vec4& params)
{
	glProgramUniform4fv(m_prog.id, m_paramsLoc, 1, prim::value_ptr(params));
}

void ProgFrame::setFrameColor(const prim::vec4& color)
{
	glProgramUniform4fv(m_prog.id, m_colorLoc, 1, prim::value_ptr(color));
}

void ProgFrame::use()
{
	glUseProgram(m_prog.id);
}
