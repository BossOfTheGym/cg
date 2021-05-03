#include "lab5-gui.h"

#include "imgui/imgui.h"

void Lab5Gui::draw()
{
	if (ImGui::Begin("lab5"))
	{
		ImGui::Text("Enjoy the projection!");
		if (ImGui::Button("Back"))
			back.emit();
	}
	ImGui::End();
}
