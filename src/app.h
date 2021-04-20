#pragma once

#include <memory>
#include <vector>

class AppState;
class MainWindow;
class AppImpl;

class App
{
public:
	App();

	App(const App&) = delete;
	App(App&&) = delete;

	~App();

	App& operator = (const App&) = delete;
	App& operator = (App&&) = delete;


public:
	void init();

	void deinit();

	void exec();

private:
	std::unique_ptr<AppImpl> m_impl;
};
