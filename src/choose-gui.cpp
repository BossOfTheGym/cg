#include "choose-gui.h"
#include "lab-options.h"

#include "imgui/imgui.h"


void ChooseGui::draw()
{
	if (ImGui::Begin("Labs"))
	{
		for (auto& option : LAB_OPTIONS)
		{
			if (ImGui::Button(option.c_str()))
				optionChosen.emit(option);
		}
		ImGui::Spacing();
		if (ImGui::Button("exit"))
			exitApp.emit();
	}
	ImGui::End();
}
