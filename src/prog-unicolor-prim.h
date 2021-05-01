#pragma once

#include "primitive.h"
#include "graphics-res/graphics-res.h"


class ProgUnicolorPrim
{
public:
	ProgUnicolorPrim();

public:
	void setProj(const prim::mat4& proj);

	void setPrimColor(const prim::vec4& color);

	void use();

private:
	res::ShaderProgram m_program;
	GLint m_colorLoc{-1};
	GLint m_projLoc{-1};
};
