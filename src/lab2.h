#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab2Impl;
}

// segments
class Lab2 : public AppState
{
	friend class Lab2Impl;

public:
	Lab2(App* app);

	~Lab2();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab2Impl> m_impl;	
};
