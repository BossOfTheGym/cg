#pragma once

#include "app-state.h"
#include "gui-signal.h"

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


private: // callbacks
	void labChosen(std::string lab);

	void exitApp();


private:
	std::unique_ptr<ChooseGui> m_gui;
	sig::Connection m_labChangedConn;
	sig::Connection m_exitApp;

	bool m_exit{false};
	bool m_chosen{false};
	std::string m_chosenLab;

	bool m_initialized{false};
};
