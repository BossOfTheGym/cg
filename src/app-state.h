#pragma once

#include <memory>
#include <string>

class App;

enum class ActionType
{
	Nothing,
	Change, // Pop prev, Push new
	Pop,
	Push,
	Exit,
};

struct AppAction
{
	ActionType type{ActionType::Nothing};
	std::string stateName;
};

// see IInputController.h
class IInputController;

// State constructor must only accept App pointer
class AppState
{
public:
	AppState(App* owner) : m_app(owner)
	{}

	virtual ~AppState() = default;

public:
	virtual bool init() = 0;

	virtual void deinit() = 0;

	virtual AppAction execute() = 0;

	// input redirected via controller from App
	virtual IInputController* getController()
	{
		return nullptr;
	}

public:
	App& app()
	{
		return *m_app;
	}

private:
	App* m_app{};
};
