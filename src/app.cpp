#include "app.h"
#include "choose-lab.h"
#include "main-window.h"
#include "gl-header/gl-header.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <cassert>


App::App()
{}

App::~App()
{
	if (m_initialized)
		deinit();
}


void App::init()
{
	if (m_initialized)
		return;

	// window init
	assert(glfw::initialize());

	using namespace glfw;

	CreationInfo info;
	info.height = 1024;
	info.width  = 1024;
	info.title = "cg";
	info.intHints.push_back({Hint::Resizable, (int)Value::False});
	info.intHints.push_back({Hint::ContextVersionMajor, 4});
	info.intHints.push_back({Hint::ContextVersionMinor, 5});
	info.intHints.push_back({Hint::DoubleBuffer, (int)Value::True});

	m_window.reset(new MainWindow(info));
	assert(m_window != nullptr);
	m_window->makeContextCurrent();

	// choose state(default state)
	auto chooseLab = std::make_unique<ChooseLab>(this);
	assert(chooseLab != nullptr);
	assert(chooseLab->init());
	m_stateStack.push_back(std::move(chooseLab));

	// imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWwindow(), false);
	ImGui_ImplOpenGL3_Init("#version 450");

	m_initialized = true;
}

void App::deinit()
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

	glfw::terminate();

	m_initialized = false;
}

void App::exec()
{
	

	m_window->makeContextCurrent();
	m_window->show();
	while (!m_window->shouldClose())
	{
		glfw::poll_events();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		m_stateStack.back()->execute();

		ImGui::Render();

		auto [w, h] = m_window->framebufferSize();
		glViewport(0, 0, w, h);
		glClearColor(1.0, 0.5, 0.25, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		m_window->swapBuffers();
	}
}
