#include "choose-gui.h"
#include "lab-options.h"

#include "imgui/imgui.h"


void ChooseGui::draw()
{
	ImGui::Begin("Labs");
	ImGui::Text("I suck baaals");
	for (auto& option : LAB_OPTIONS)
	{
		if (ImGui::Button(option.c_str()))
			optionChosen.emit(option);
	}
	ImGui::End();
}
