#include "state-register.h"

#include <cassert>

namespace
{
	std::unique_ptr<NameFactory> allStates;

	NameFactory& get_states()
	{
		if (allStates == nullptr)
			allStates = std::make_unique<NameFactory>();
		return *allStates;
	}
}

const NameFactory& AllStates::states()
{
	return get_states();
}

AppState* AllStates::create(const Name& name, App* app)
{
	auto& all = get_states();
	auto it = all.find(name); 
	assert(it != all.end() && "State with given name not found");
	return it->second->create(app);
}

void AllStates::register_state(Name name, FactoryPtr factory)
{
	auto& all = get_states();
	auto [it, wasInserted] = all.insert({name, std::move(factory)});
	assert(wasInserted && "State with given name already exists.");
}
