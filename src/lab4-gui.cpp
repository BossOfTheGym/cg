#include "lab4-gui.h"

#include "imgui/imgui.h"

using namespace std::string_literals;

Lab4Gui::Lab4Gui(u32 maxPoints, u32 initPoints) 
	: m_maxPoints(maxPoints)
	, m_currPoints(initPoints)
{
	m_maxPointString = "max points: " + std::to_string(maxPoints);
}

void Lab4Gui::draw()
{
	if (ImGui::Begin("lab3"))
	{
		ImGui::Text(m_maxPointString.c_str());
		if (!m_built)
		{
			if (ImGui::InputScalar("points", ImGuiDataType_U32, &m_currPoints))
				m_currPoints = std::min(m_maxPoints, m_currPoints);

			if (ImGui::Button("Generate"))
			{
				m_generated = true;
				generate.emit(m_currPoints);
			}

			if (m_generated)
			{
				if (ImGui::Button("Build")) // gui user must call hullWasBuilt
				{
					m_generated = false;
					build.emit();
				}
			}
		}
		else
		{
			ImGui::Text(m_currPointsString.c_str());
			if (ImGui::Button("Clear"))
			{
				clearBuiltTriang();
				clear.emit();
			}
		}

		if (ImGui::Button("Back"))
		{
			back.emit();
		}
	}
	ImGui::End();
}

void Lab4Gui::triangWasBuilt()
{
	m_built = true;
	m_currPointsString = "points: " + std::to_string(m_currPoints);
}

void Lab4Gui::clearBuiltTriang()
{
	m_built = false;
	m_currPointsString.clear();
}
