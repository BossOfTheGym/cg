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
	uniform mat4 proj;

	// produce:
	// v0 : (x0, y0)
	// v1 : (x0, y1)
	// v2 : (x1, y1)
	// v3 : (x1, y0)
	void main()
	{
		float x = params[gl_VertexID / 2];
		float y = params[2 + (1 + gl_VertexID) / 2 % 2];
		gl_Position = proj * vec4(x, y, 0.0, 1.0);
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
	bool stat{};

	res::Shader vert;
	stat = res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE);
	assert(stat);

	res::Shader frag;
	stat = res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE);
	assert(stat);

	assert(res::try_create_shader_program(m_prog, vert, frag));

	m_paramsLoc = glGetUniformLocation(m_prog.id, "params");
	assert(m_paramsLoc != -1);
	m_colorLoc = glGetUniformLocation(m_prog.id, "frameColor");
	assert(m_colorLoc != -1);
	m_projLoc = glGetUniformLocation(m_prog.id, "proj");
	assert(m_projLoc != -1);
}


void ProgFrame::setProj(const prim::mat4& proj)
{
	glProgramUniformMatrix4fv(m_prog.id, m_projLoc, 1, GL_FALSE, prim::value_ptr(proj));
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
