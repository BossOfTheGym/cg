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

// State constructor must only accept App pointer
class AppState
{
public:
	AppState(App* owner) : m_app(owner)
	{}

	virtual ~AppState() = default;

public:
	virtual bool init() { return true; };

	virtual void deinit() {};

	virtual AppAction execute() { return {}; };

	virtual void pause() {}

	virtual void resume() {}


public:
	App& app()
	{
		return *m_app;
	}

private:
	App* m_app{};
};
