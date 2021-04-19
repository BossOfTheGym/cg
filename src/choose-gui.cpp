#include "choose-gui.h"

#include "imgui/imgui.h"


void ChooseGui::draw()
{
	bool showDemoWindow = true;
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);
}

auto ChooseGui::optionChosen() -> OptionChosen&
{
	return m_optionChosen;
}
