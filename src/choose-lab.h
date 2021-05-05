#pragma once

#include "app-state.h"
#include "gui-signal.h"

#include <string>


namespace
{
	class ChooseGuiImpl;
}

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

	virtual void resume() override;

	virtual void pause() override;

private:
	std::unique_ptr<ChooseGuiImpl> m_impl;
};
