#pragma once

#include "primitive.h"
#include "graphics-res/graphics-res.h"


// draw any primitives that have position and color attributes
// primitive : any primitive
// attributes : pos - vec2, color - vec4
class ProgColorPrim
{
public:
	ProgColorPrim();

public:
	void setProj(const prim::mat4& proj);

	void use();

private:
	res::ShaderProgram m_prog;
	GLint m_projLoc{-1};
};
