#include "lab1-gui.h"

#include "imgui/imgui.h"

#include <utility>

Lab1Gui::Lab1Gui(u32 maxPoints, u32 initPoints) 
	: m_maxPoints(maxPoints)
	, m_currPoints(initPoints)
{
	m_maxPointsString = "Max points: " + std::to_string(maxPoints);
}

void Lab1Gui::draw()
{
	if (ImGui::Begin("lab1"))
	{
		ImGui::Text(m_maxPointsString.c_str());

		if (ImGui::InputScalar("count", ImGuiDataType_U32, &m_currPoints))
			m_currPoints = std::min(m_currPoints, m_maxPoints);

		if (ImGui::Button("Generate"))
			generatePoints.emit(m_currPoints);

		if (ImGui::InputFloat4("x0 x1 y0 y1", m_params))
		{
			if (m_params[0] > m_params[1])
				std::swap(m_params[0], m_params[1]);
			if (m_params[2] > m_params[3])
				std::swap(m_params[2], m_params[3]);
			frameParamsChanged.emit(m_params[0], m_params[1], m_params[2], m_params[3]);
		}

		if (ImGui::Button("Back"))
			returnBack.emit();
	}
	ImGui::End();
}
