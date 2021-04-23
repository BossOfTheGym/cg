#include "lab5-gui.h"

#include "imgui/imgui.h"

void Lab5Gui::draw()
{
	if (ImGui::Begin("lab5"))
	{
		ImGui::Text("this lab really must be tested");
		if (ImGui::Button("Back"))
			back.emit();
	}
	ImGui::End();
}
