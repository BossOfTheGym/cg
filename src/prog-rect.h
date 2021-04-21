#pragma once

#include "primitive.h"
#include "graphics-res/graphics-res.h"

// draws NDC rect, requires dummy VertexArray to operate
// it serves as non-API way to copy one texture to another(textures are attached to framebuffers)
// requires 4 vertices
// primitive : GL_TRIANGLE_STRIP
class ProgRect
{
public:
	ProgRect();

public:
	void setTexture(GLuint texId);

	void use();

private:
	res::ShaderProgram m_prog;
};
