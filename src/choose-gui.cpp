#include "choose-gui.h"
#include "state-register.h"

#include "imgui/imgui.h"


void ChooseGui::draw()
{
	if (ImGui::Begin("Labs"))
	{
		for (auto& [name, _] : AllStates::states())
		{
			if (name == "choose-lab")
				continue; // workaround :)

			if (ImGui::Button(name.c_str()))
				optionChosen.emit(name);
		}
		ImGui::Spacing();
		if (ImGui::Button("exit"))
			exitApp.emit();
	}
	ImGui::End();
}
