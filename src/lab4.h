#pragma once

#include "app-state.h"

#include <memory>


namespace
{
	class Lab4Impl;
}

class Lab4 : public AppState
{
	friend class Lab4Impl;

public:
	Lab4(App* app);

	~Lab4();

public: // AppState
	virtual bool init() override;

	virtual void deinit() override;

	virtual AppAction execute() override;


private:
	std::unique_ptr<Lab4Impl> m_impl;	
};
