#pragma once

#include <memory>
#include <vector>

class AppState;
class MainWindow;

class App
{
public:
	App();

	~App();


public:
	void init();

	void deinit();

	void exec();


private:
	std::vector<std::unique_ptr<AppState>> m_stateStack;
	std::unique_ptr<MainWindow> m_window;
	bool m_initialized{false};
};
