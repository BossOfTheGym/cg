#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

#include <string>

// segments
class Lab2Gui : public Gui
{
public:
	using Generate     = sig::Signal<void(u32)>;
	using Intersect    = sig::Signal<void()>;
	using Clear        = sig::Signal<void()>;
	using Back         = sig::Signal<void()>;

public:
	Lab2Gui(u32 maxSegmentCount, u32 initCount);

public:
	virtual void draw() override;

public: // state transtion
	void setIntersectionInfo(u32 intersections);

private:
	void clearIntersectionInfo();

public: // signals
	Generate     generate;
	Intersect    intersect;
	Clear        clear;
	Back         back;

private:
	u32 m_maxSegmentCount{};
	u32 m_segmentCount{};
	std::string m_maxSegmentsString;

	// generated
	bool m_generated{};

	// intersected
	bool m_intersected{};
	u32 m_intersections{};
	std::string m_intersectionString;
};
