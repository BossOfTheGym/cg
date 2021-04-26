#include "app.h"
#include "choose-lab.h"
#include "lab-options.h"
#include "main-window.h"
#include "state-creator.h"
#include "gl-header/gl-header.h"

#include "graphics-res/graphics-res.h"
#include "graphics-res/graphics-res-util.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <string>
#include <cassert>
#include <iostream>


class AppImpl
{
public:
	AppImpl(App* app) : m_app(app)
	{}

	AppImpl(const AppImpl&) = delete;
	AppImpl(AppImpl&&)      = delete;

	~AppImpl()
	{
		if (m_initialized)
			deinit();
	}

	AppImpl& operator = (const AppImpl&) = delete;
	AppImpl& operator = (AppImpl&&)      = delete;


public:
	void init()
	{
		if (m_initialized)
			return;

		// glfw
		bool stat = glfw::initialize();
		assert(stat);

		using namespace glfw;

		// window init
		CreationInfo info;
		info.height = 1024;
		info.width  = 1280;
		info.title = "cg";
		info.intHints.push_back({Hint::Resizable, (int)Value::False});
		info.intHints.push_back({Hint::ContextVersionMajor, 4});
		info.intHints.push_back({Hint::ContextVersionMinor, 5});
		info.intHints.push_back({Hint::DoubleBuffer, (int)Value::True});

		m_window.reset(new MainWindow(info));
		assert(m_window != nullptr);
		m_window->makeContextCurrent();

		// choose state(default state)
		auto chooseLab = std::make_unique<ChooseLab>(m_app);
		assert(chooseLab != nullptr);
		stat = chooseLab->init();
		assert(stat);

		m_stateStack.push_back(std::move(chooseLab));

		// imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWwindow(), true);
		ImGui_ImplOpenGL3_Init("#version 450 core");

		m_initialized = true;
	}

	void deinit()
	{
		if (!m_initialized)
			return;

		// imgui
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// states
		while(!m_stateStack.empty())
		{
			m_stateStack.back()->deinit();
			m_stateStack.pop_back();
		}

		// window
		m_window.reset();

		// glfw
		glfw::terminate();

		m_initialized = false;
	}

	void exec()
	{
		m_window->makeContextCurrent();
		m_window->show();
		while (!m_window->shouldClose())
		{
			if (m_stateStack.empty())
				break;

			glfw::poll_events();

			glClearColor(1.0, 0.5, 0.25, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			startGuiFrame();

			handleAction(m_stateStack.back()->execute());

			// demo
			ImGui::ShowDemoWindow();

			endGuiFrame();
			drawGui();

			m_window->swapBuffers();
		}
	}

public:
	MainWindow* window()
	{
		return m_window.get();
	}

private: // action handling
	void handleAction(AppAction action)
	{
		switch(action.type)
		{
			case ActionType::Exit:
				m_window->shouldClose(true);
				break;

			case ActionType::Pop:
				m_stateStack.back()->deinit();
				m_stateStack.pop_back();

				m_stateStack.back()->resume();
				break;

			case ActionType::Push:
				m_stateStack.back()->pause();

				auto newState = m_stateCreator.createState(m_app, action.stateName);
				assert(newState != nullptr);
				bool stat = newState->init();
				assert(stat);

				m_stateStack.push_back(std::move(newState));
				break;
		}
	}

private: // gui
	void startGuiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void endGuiFrame()
	{
		ImGui::Render();
	}

	void drawGui()
	{
		auto [w, h] = m_window->framebufferSize();
		glViewport(0, 0, w, h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	App* m_app{};
	bool m_initialized{false};

	std::vector<std::unique_ptr<AppState>> m_stateStack;
	std::unique_ptr<MainWindow> m_window;
	StateCreator m_stateCreator;
};


// App
App::App()
{
	m_impl.reset(new AppImpl(this));
}

App::~App()
{
	m_impl.reset();
}

void App::init()
{
	m_impl->init();
}

void App::deinit()
{
	m_impl->deinit();
}

void App::exec()
{
	m_impl->exec();
}

MainWindow* App::window()
{
	return m_impl->window();
}
