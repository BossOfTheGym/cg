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
	m_labChangedConn = m_gui->optionChosen.connect([&](std::string lab){ labChosen(std::move(lab)); });
	m_exitApp        = m_gui->exitApp     .connect([&](){ exitApp(); });

	m_initialized = true;
	
	return true;
}

void ChooseLab::deinit()
{
	if (!m_initialized)
		return;

	m_labChangedConn.release();
	m_exitApp.release();
	m_gui.reset();
	m_chosen = false;
	m_chosenLab.clear();

	m_initialized = false;
}

AppAction ChooseLab::execute()
{
	m_gui->draw();

	if (m_exit)
	{
		return AppAction{ActionType::Exit};
	}
	if (m_chosen)
	{
		m_chosen = false;
		return AppAction{ActionType::Push, std::move(m_chosenLab)};
	}

	return AppAction{};
}

// callbacks
void ChooseLab::labChosen(std::string lab)
{
	m_chosen = true;
	m_chosenLab = std::move(lab);
}

void ChooseLab::exitApp()
{
	m_exit = true;
}
