#pragma once

#include "primitive.h"
#include "graphics-res/graphics-res.h"

// requires dummy VertexArray
// draws 4 vertices
// primitive : GL_LINE_LOOP
class ProgFrame
{
public:
	ProgFrame();

public:
	void setProj(const prim::mat4& proj);

	// vec4 : x - x0, y - x1, z - y0, w - y1
	void setFrameParams(const prim::vec4& v0);

	void setFrameColor(const prim::vec4& color);

	void use();

private:
	res::ShaderProgram m_prog;
	GLint m_paramsLoc{-1};
	GLint m_colorLoc{-1};
	GLint m_projLoc{-1};
};
