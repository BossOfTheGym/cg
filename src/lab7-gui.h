#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

#include "bezier.h"

class Lab7Gui : public Gui
{
public:
	// index, x, y, z
	using VertexChanged = sig::Signal<void(u32, f32, f32, f32)>;

public:
	virtual void draw() override;

public: // signals
	VertexChanged vertexChanged;

public:
	void setVertex(u32 i, f32)

private:
	f32 m_min{};
	f32 m_max{};
	f32 m_vert[16][3];
};
