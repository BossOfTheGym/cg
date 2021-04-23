#include "state-creator.h"

#include "lab1.h"
#include "lab2.h"
#include "lab3.h"
#include "lab4.h"
#include "lab5.h"
#include "lab6.h"

#include <cassert>

template<class State>
class StateFactory : public IStateFactory
{
public:
	virtual AppState* create(App* app) override
	{
		return new State(app);
	}
};

StateCreator::StateCreator()
{
	m_factories["lab1"] = std::make_unique<StateFactory<Lab1>>();
	m_factories["lab2"] = std::make_unique<StateFactory<Lab2>>();
	m_factories["lab3"] = std::make_unique<StateFactory<Lab3>>();
	m_factories["lab4"] = std::make_unique<StateFactory<Lab4>>();
	m_factories["lab5"] = std::make_unique<StateFactory<Lab5>>();
	m_factories["lab6"] = std::make_unique<StateFactory<Lab6>>();
}

std::unique_ptr<AppState> StateCreator::createState(App* app, const std::string& name)
{
	if (auto it = m_factories.find(name); it != m_factories.end())
	{
		return std::unique_ptr<AppState>(it->second->create(app));
	}
	return nullptr;
}
