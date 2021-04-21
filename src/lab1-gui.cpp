#include "lab1-gui.h"

#include "imgui/imgui.h"

#include <utility>

Lab1Gui::Lab1Gui(u32 maxPoints) : m_maxPoints(maxPoints)
{}

void Lab1Gui::draw()
{
	if (ImGui::Begin("lab1"))
	{
		ImGui::Text("max points: %u", m_maxPoints);

		if (ImGui::InputScalar("gen count", ImGuiDataType_U32, &m_currPoints, nullptr, nullptr, "%u"))
			m_currPoints = std::min(m_currPoints, m_maxPoints);

		if (ImGui::Button("generate"))
			generatePoints.emit(m_currPoints);
	}
	ImGui::End();
}
