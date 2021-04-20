#pragma once

#include <memory>
#include <string>

class App;

struct AppAction
{
	enum Type
	{
		Nothing,
		Change,
		Exit,
	};

	Type type{Nothing};
	std::string option;
};

class AppState
{
public:
	AppState(App* owner) : m_app(owner)
	{}

public:
	virtual bool init() = 0;

	virtual void deinit() = 0;

	virtual AppAction execute() = 0;

public:
	App& app()
	{
		return *m_app;
	}

private:
	App* m_app{};
};
