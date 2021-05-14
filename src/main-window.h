#pragma once

#include "gui-signal.h"
#include "glfw-cxx/glfw3.h"

class MainWindow : public glfw::Window
{
public:
	using MouseMoved  = sig::Signal<void(double, double)>;
	using MouseButton = sig::Signal<void(int, int, int)>;
	using Scrolled    = sig::Signal<void(double, double)>;
public:
	MainWindow(const glfw::CreationInfo& info);	


private: // glfw::Window
	virtual void mouseMoveEvent(double xpos, double ypos) override;

	virtual void mouseButtonEvent(int button, int action, int mods) override;

	virtual void scrollEvent(double xOffset, double yOffset) override;

public: // signals
	MouseMoved  mouseMoved;
	MouseButton mouseButton;
	Scrolled scrolled;

	void disconnectAll();
};
