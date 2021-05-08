#pragma once

#include <map>
#include <memory>
#include <string>

class App;
class AppState;

struct IStateFactory
{
	virtual ~IStateFactory() = default;

	virtual AppState* create(App* app) = 0;
};

template<class State>
struct StateFactory : IStateFactory
{
	virtual AppState* create(App* app) override
	{
		return new State(app);
	}
};

namespace impl
{
	template<class State>
	struct Register;
}

using Name = std::string;
using FactoryPtr  = std::unique_ptr<IStateFactory>;
using NameFactory = std::map<Name, FactoryPtr>;

class AllStates
{
	template<class State>
	friend struct impl::Register;

public:
	static const NameFactory& states();

	static AppState* create(const Name& name, App* app);

private:
	static void register_state(Name name, FactoryPtr factory);
};


#define REGISTER_STATE(stateName, stateClass)                                                    \
namespace impl                                                                                   \
{                                                                                                \
	template<>                                                                                   \
	struct Register<stateClass>                                                                  \
	{                                                                                            \
		Register()                                                                               \
		{                                                                                        \
			AllStates::register_state(#stateName, std::make_unique<StateFactory<stateClass>>()); \
		}                                                                                        \
	};                                                                                           \
	                                                                                             \
	static Register<stateClass> reg##stateClass;                                                 \
}
