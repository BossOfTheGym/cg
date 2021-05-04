#include "lab6-gui.h"

#include "imgui/imgui.h"

void Lab6Gui::draw()
{
	if (ImGui::Begin("lab6"))
	{
		ImGui::Text("Enjoy bezier patch!");
		if (ImGui::Button("Back"))
			back.emit();
	}
	ImGui::End();
}