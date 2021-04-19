#pragma once

#include "gui-signal.h"
#include "glfw-cxx/glfw3.h"

class MainWindow : public glfw::Window
{
public:
	using MouseMoved  = sig::Signal<void(double, double)>;
	using MouseButton = sig::Signal<void(int, int, int)>;

public:
	MainWindow(const glfw::CreationInfo& info);	


private: // glfw::Window
	virtual void mouseMoveEvent(double xpos, double ypos);

	virtual void mouseButtonEvent(int button, int action, int mods);


public: // signals
	MouseMoved  mouseMoved;
	MouseButton mouseButton;

	void disconnectAll();
};
