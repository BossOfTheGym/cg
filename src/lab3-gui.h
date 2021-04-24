#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

#include <string>

// hull
// generate
// build
// clear
class Lab3Gui : public Gui
{
public:
	using Generate	    = sig::Signal<void(u32)>;
	using Build		    = sig::Signal<void()>;
	using Clear		    = sig::Signal<void()>;
	using Back		    = sig::Signal<void()>;

public:
	Lab3Gui(u32 maxPoints, u32 initPoints);

public:
	virtual void draw() override;

public: // signals
	Generate     generate;
	Build        build;
	Clear        clear;
	Back         back;

public:
	void hullWasBuilt();

private:
	void clearBuiltHull();

private:
	u32 m_maxPoints{};
	u32 m_currPoints{};
	std::string m_maxPointString;

	bool m_generated{};

	// built
	bool m_built{};
	std::string m_currPointsString;
};
