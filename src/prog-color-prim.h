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
	void use();

private:
	res::ShaderProgram m_prog;
};
