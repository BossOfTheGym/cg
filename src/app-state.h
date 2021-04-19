#pragma once

#include <memory>

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
