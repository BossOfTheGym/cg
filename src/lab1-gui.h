#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

#include <string>

// quadtree
class Lab1Gui : public Gui
{
public:
	using GeneratePoints     = sig::Signal<void(u32)>;
	using ReturnBack         = sig::Signal<void()>;
	using FrameParamsChanged = sig::Signal<void(f32, f32, f32, f32)>;

	Lab1Gui(u32 maxPoints, u32 initPoints);

public:
	virtual void draw() override;

public: // signals
	GeneratePoints     generatePoints;
	FrameParamsChanged frameParamsChanged;
	ReturnBack         returnBack;

private:
	u32 m_maxPoints{};
	u32 m_currPoints{};
	f32 m_params[4]{};

	std::string m_maxPointsString;
};
