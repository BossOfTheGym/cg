#include "prog-rect.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>


namespace
{
	inline const char* VERT_SOURCE = R"vert(
	#version 450 core

	const vec2 verts[4] = 
	{
		vec2(-1.0, +1.0),
		vec2(-1.0, -1.0),
		vec2(+1.0, +1.0),
		vec2(+1.0, -1.0),
	};

	const vec2 uvs[4] = 
	{
		vec2(0.0, 1.0),
		vec2(0.0, 0.0),
		vec2(1.0, 1.0),
		vec2(1.0, 0.0),
	};

	out vec2 uv;

	void main()
	{
		gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
		uv = uvs[gl_VertexID];
	}
	)vert";

	inline const char* FRAG_SOURCE = R"frag(
	#version 450 core

	out vec4 color;
	in vec2 uv;

	layout(binding = 0) uniform sampler2D tex;

	void main()
	{
		color = texture(tex, uv);
	}
	)frag";

	constexpr const GLuint TEX_BINDING = 0;
}

ProgRect::ProgRect()
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
}

void ProgRect::setTexture(GLuint texId)
{
	glBindTextureUnit(TEX_BINDING, texId);
}

void ProgRect::use()
{
	glUseProgram(m_prog.id);
}
