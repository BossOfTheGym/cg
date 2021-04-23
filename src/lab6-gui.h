#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

class Lab6Gui : public Gui
{
public:
	using Back = sig::Signal<void()>;

public:
	virtual void draw() override;

public: // signals
	Back back;
};
