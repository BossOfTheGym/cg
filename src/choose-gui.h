#pragma once

#include "gui.h"
#include "core.h"
#include "gui-signal.h"

#include <memory>
#include <string>

class AppState;

class ChooseGui : public Gui
{
public:
	using OptionChosen = sig::Signal<void(std::string)>;

public:
	ChooseGui() = default;
	
	ChooseGui(const ChooseGui&) = delete;
	ChooseGui(ChooseGui&&) = delete;

	~ChooseGui() = default;

	ChooseGui& operator = (const ChooseGui&) = delete;
	ChooseGui& operator = (ChooseGui&&) = delete;

public: // Gui
	virtual void draw() override;

public: // signals
	OptionChosen optionChosen;
};
