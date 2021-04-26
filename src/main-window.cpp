#include "main-window.h"

#include "imgui/imgui.h"


MainWindow::MainWindow(const glfw::CreationInfo& info) : glfw::Window(info)
{}

void MainWindow::mouseMoveEvent(double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		mouseMoved.emit(xpos, ypos);
}

void MainWindow::mouseButtonEvent(int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
		mouseButton.emit(button, action, mods);
}

void MainWindow::disconnectAll()
{
	mouseMoved.disconnectAll();
	mouseButton.disconnectAll();
}
