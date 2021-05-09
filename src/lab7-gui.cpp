#include "lab7-gui.h"

#include "imgui/imgui.h"

#include <cstdio>
#include <cassert>

Lab7Gui::Lab7Gui(f32 speed, f32 min, f32 max, Patch* controlled) 
	: m_speed{speed}, m_min(min), m_max(max), m_controlled(controlled)
{
	assert(controlled != nullptr);
}

void Lab7Gui::draw()
{
	if (ImGui::Begin("lab7"))
	{
		ImGui::Text("Control the bezier patch!");
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				auto id = i * 4 + j;

				char vid[4]{};
				snprintf(vid, 4, "v%d", id);

				ImGui::PushID(id);
				if (ImGui::DragFloat3(vid, prim::value_ptr(m_controlled->vs[id]), m_speed, m_min, m_max))
					vertexChanged.emit(id);
				ImGui::PopID();
			}
			if (i != 3)
				ImGui::Separator();
		}
		if (ImGui::Button("Back"))
			returnBack.emit();
	}
	ImGui::End();
}