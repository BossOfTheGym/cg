#include "choose-lab.h"
#include "choose-gui.h"
#include "gl-header/gl-header.h"

ChooseLab::ChooseLab(App* owner) : AppState(owner)
{}

ChooseLab::~ChooseLab()
{
	if (m_initialized)
		deinit();
}

// AppState
bool ChooseLab::init()
{
	if (m_initialized)
		return true;

	m_gui.reset(new ChooseGui());
	m_gui->optionChosen.connect([&](auto lab)
	{
		labChosen(std::move(lab));
	});
	m_initialized = true;
	
	return true;
}

void ChooseLab::deinit()
{
	if (!m_initialized)
		return;

	m_gui.reset();
	m_initialized = false;
	m_chosen = false;
	m_chosenLab.clear();
}

AppAction ChooseLab::execute()
{
	m_gui->draw();
	if (m_chosen)
	{
		// TODO
	}

	return AppAction{};
}

void ChooseLab::labChosen(std::string lab)
{
	//m_chosen = true;
	m_chosenLab = std::move(lab);
}
