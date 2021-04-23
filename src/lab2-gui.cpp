#include "lab2-gui.h"

#include "imgui/imgui.h"

using namespace std::string_literals;

Lab2Gui::Lab2Gui(u32 maxSegmentsCount) : m_maxSegmentCount(maxSegmentsCount)
{
	m_maxSegmentsString = "max segments: "s + std::to_string(maxSegmentsCount);
}

void Lab2Gui::draw()
{
	if (ImGui::Begin("lab2"))
	{
		if (!m_intersected)
		{
			ImGui::Text(m_maxSegmentsString.c_str());

			if (ImGui::InputScalar("Segments count", ImGuiDataType_U32, &m_segmentCount))
			{
				m_segmentCount = std::min(m_segmentCount, m_maxSegmentCount);
				countChanged.emit(m_segmentCount);
			}

			if (ImGui::Button("Generate"))
			{
				generate.emit(m_segmentCount);
				m_generated = true;
			}

			if (m_generated)
			{
				if (ImGui::Button("Intersect"))
				{
					intersect.emit(); // gui user should call setIntersectionInfo afterwards
					m_generated = false;
				}
			}
		}
		else // after setIntersectionInfo has been called
		{
			ImGui::Text(m_intersectionString.c_str());
			if (ImGui::Button("Clear"))
			{
				clearIntersectionInfo();
				clear.emit();
			}
		}
		if (ImGui::Button("Back"))
			back.emit();
	}
	ImGui::End();
}

void Lab2Gui::setIntersectionInfo(u32 intersections)
{
	m_intersected = true;
	m_intersections = intersections;
	if (intersections != 0)
		m_intersectionString = "Intersections: "s + std::to_string(intersections);
	else
		m_intersectionString = "No intersections";
}

void Lab2Gui::clearIntersectionInfo()
{
	m_intersected = false;
	m_intersections = 0;
	m_intersectionString.clear();
}
