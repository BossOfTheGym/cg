#pragma once

#include <map>
#include <memory>
#include <string>

class App;
class AppState;

class IStateFactory
{
public:
	virtual AppState* create(App* app) = 0;
};

class StateCreator
{
public:
	StateCreator();

public:
	std::unique_ptr<AppState> createState(App* app, const std::string& name);

private:
	std::map<std::string, std::unique_ptr<IStateFactory>> m_factories;
};
