#pragma once

#include "gui.h"
#include "app-state.h"

#include <string>

class ChooseGui;

class ChooseLab : public AppState
{
public:
	ChooseLab(App* app);
	~ChooseLab();

	ChooseLab(const ChooseLab&) = delete;
	ChooseLab(ChooseLab&&)      = delete;

	ChooseLab& operator = (const ChooseLab&) = delete;
	ChooseLab& operator = (ChooseLab&&)      = delete;


public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	void labChosen(std::string lab);

private:
	std::unique_ptr<ChooseGui> m_gui;
	bool m_initialized{false};
	bool m_chosen{};
	std::string m_chosenLab;
};
