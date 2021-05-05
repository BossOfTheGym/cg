#pragma once

#include "primitive.h"
#include "graphics-res/graphics-res.h"

class ProgFractal
{
public:
	ProgFractal();

public:
	void use();

	void setResolution(const prim::vec2& res);

	void setTime(f32 t);

private:
	res::ShaderProgram m_prog;
	GLint m_resolutionLoc{-1};
	GLint m_timeLoc{-1};
};
