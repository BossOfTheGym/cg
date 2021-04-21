#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab1Impl;
}

class Lab1 : public AppState
{
	friend class Lab1Impl;

public:
	Lab1(App* app);

	~Lab1();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab1Impl> m_impl;	
};
