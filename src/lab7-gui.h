#pragma once

#include "gui.h"
#include "core.h"
#include "bezier.h"
#include "primitive.h"
#include "gui-signal.h"

class Lab7Gui : public Gui
{
public:
	using Patch = bezier::Patch3D<prim::vec3>;

	using VertexChanged = sig::Signal<void(u32)>;
	using ReturnBack    = sig::Signal<void()>;

public:
	Lab7Gui(f32 speed, f32 min, f32 max, Patch* controlled);

public:
	virtual void draw() override;

public: // signals
	VertexChanged vertexChanged;
	ReturnBack    returnBack;

private:
	f32 m_speed{};
	f32 m_min{};
	f32 m_max{};
	Patch* m_controlled{};
};
