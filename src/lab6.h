#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab6Impl;
}

class Lab6 : public AppState
{
	friend class Lab6Impl;

public:
	Lab6(App* app);

	~Lab6();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab6Impl> m_impl;	
};
