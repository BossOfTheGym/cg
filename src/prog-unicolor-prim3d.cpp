#include "prog-unicolor-prim3d.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>


namespace
{
	const char* VERT_SOURCE = R"vert(
	#version 450 core

	layout(location = 0) in vec3 pos;

	uniform mat4 proj;

	void main()
	{
		gl_Position = proj * vec4(pos, 1.0);
	}
	)vert";

	inline const char* FRAG_SOURCE = R"frag(
	#version 450 core

	out vec4 color;

	uniform vec4 primColor;

	void main()
	{
		color = primColor;
	}
	)frag";
}


ProgUnicolorPrim3D::ProgUnicolorPrim3D()
{
	bool stat{};

	res::Shader vert;
	stat = res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE);
	assert(stat);

	res::Shader frag;
	stat = res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE);
	assert(stat);

	stat = res::try_create_shader_program(m_program, vert, frag);
	assert(stat);

	m_projLoc = glGetUniformLocation(m_program.id, "proj");
	assert(m_projLoc != -1);
	m_colorLoc = glGetUniformLocation(m_program.id, "primColor");
	assert(m_colorLoc != -1);
}

void ProgUnicolorPrim3D::setProj(const prim::mat4& proj)
{
	glProgramUniformMatrix4fv(m_program.id, m_projLoc, 1, GL_FALSE, prim::value_ptr(proj));
}

void ProgUnicolorPrim3D::setPrimColor(const prim::vec4& color)
{
	glProgramUniform4fv(m_program.id, m_colorLoc, 1, prim::value_ptr(color));
}

void ProgUnicolorPrim3D::use()
{
	glUseProgram(m_program.id);
}

