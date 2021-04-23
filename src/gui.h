#pragma once

class Gui
{
public:
	virtual ~Gui() = default;

public:
	virtual void draw() = 0;
};