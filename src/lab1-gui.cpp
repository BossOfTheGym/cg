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

		if (ImGui::InputScalar("gen count", ImGuiDataType_U32, &m_currPoints))
			m_currPoints = std::min(m_currPoints, m_maxPoints);

		if (ImGui::Button("generate"))
			generatePoints.emit(m_currPoints);

		if (ImGui::InputFloat4("x0 x1 y0 y1", m_params))
		{
			if (m_params[0] > m_params[1])
				std::swap(m_params[0], m_params[1]);
			if (m_params[2] > m_params[3])
				std::swap(m_params[2], m_params[3]);
			frameParamsChanged.emit(m_params[0], m_params[1], m_params[2], m_params[3]);
		}

		if (ImGui::Button("back"))
			returnBack.emit();
	}
	ImGui::End();
}
