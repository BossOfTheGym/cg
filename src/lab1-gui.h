#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"


class Lab1Gui : public Gui
{
public:
	using GeneratePoints = sig::Signal<void(u32)>;

	Lab1Gui(u32 maxPoints);

public:
	virtual void draw() override;

public: // signals
	GeneratePoints generatePoints;

private:
	u32 m_maxPoints{};
	u32 m_currPoints{};
};
