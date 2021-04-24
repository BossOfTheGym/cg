#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab3Impl;
}

class Lab3 : public AppState
{
	friend class Lab3Impl;

public:
	Lab3(App* app);

	~Lab3();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab3Impl> m_impl;	
};
