#include "choose-gui.h"
#include "lab-options.h"

#include "imgui/imgui.h"


void ChooseGui::draw()
{
	if (ImGui::Begin("Labs"))
	{
		ImGui::Text("I suck baaals");
		for (auto& option : LAB_OPTIONS)
		{
			if (ImGui::Button(option.c_str()))
				optionChosen.emit(option);
		}	
	}
	ImGui::End();

	// test lab1(quadtree)
	if (ImGui::Begin("test lab1"))
	{
		ImGui::Text("max points: %u", m_maxPoints);

		if (ImGui::InputScalar("gen count", ImGuiDataType_U32, &m_currPoints, nullptr, nullptr, "%u"))
			m_currPoints = std::min(m_currPoints, m_maxPoints);

		if (ImGui::Button("generate"));	
	}
	ImGui::End();

	// test lab2(segments)
}
