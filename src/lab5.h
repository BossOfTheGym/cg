#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab5Impl;
}

class Lab5 : public AppState
{
	friend class Lab5Impl;

public:
	Lab5(App* app);

	~Lab5();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab5Impl> m_impl;	
};
