#include "main-window.h"

MainWindow::MainWindow(const glfw::CreationInfo& info) : glfw::Window(info)
{}

void MainWindow::mouseMoveEvent(double xpos, double ypos)
{
	mouseMoved.emit(xpos, ypos);
}

void MainWindow::mouseButtonEvent(int button, int action, int mods)
{
	mouseButton.emit(button, action, mods);
}

void MainWindow::disconnectAll()
{
	mouseMoved.disconnectAll();
	mouseButton.disconnectAll();
}
