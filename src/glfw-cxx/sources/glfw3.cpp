#include "../glfw3.h"
#include "../opengl-loader.h"


// window
namespace glfw
{
	// callbacks
	void Window::dispatch_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->keyPressEvent(key, scancode, action, mods);
	}

	void Window::dispatch_char_callback(GLFWwindow* window, unsigned int codepoint)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->charEvent(codepoint);
	}

	void Window::dispatch_cursor_pos_callback(GLFWwindow* window, double xPos, double yPos)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->mouseMoveEvent(xPos, yPos);
	}

	void Window::dispatch_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->mouseButtonEvent(button, action, mods);
	}

	void Window::dispatch_cursor_enter_callback(GLFWwindow* window, int entered)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->mouseEnterEvent(entered);
	}

	void Window::dispatch_scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->scrollEvent(xOffset, yOffset);
	}

	void Window::dispatch_drop_callback(GLFWwindow* window, int pathCount, const char* paths[])
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->dropEvent(pathCount, paths);
	}


	void Window::dispatch_window_close_callback(GLFWwindow* window)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->closeEvent();
	}

	void Window::dispatch_window_size_callback(GLFWwindow* window, int width, int height)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->resizeEvent(width, height);
	}

	void Window::dispatch_framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->framebufferResizeEvent(width, height);
	}

	void Window::dispatch_window_pos_callback(GLFWwindow* window, int xPos, int yPos)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->moveEvent(xPos, yPos);
	}

	void Window::dispatch_window_iconify_callback(GLFWwindow* window, int iconified)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->minimizeEvent(iconified);
	}

	void Window::dispatch_window_maximize_callback(GLFWwindow* window, int maximized)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->maximizeEvent(maximized);
	}

	void Window::dispatch_window_focus_callback(GLFWwindow* window, int focused)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->focusEvent(focused);
	}

	void Window::dispatch_window_refresh_callback(GLFWwindow* window)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->refreshEvent();
	}

	void Window::dispatch_window_content_scale_callback(GLFWwindow* window, float xScale, float yScale)
	{
		Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
		wnd->contentScaleEvent(xScale, yScale);
	}


	// contructors & destructor
	Window::Window()
	{}

	Window::Window(
		const CreationInfo& info
		, const Window& contextToShare)
	{
		for (auto& [hint, value] : info.intHints)
		{
			glfwWindowHint((int)hint, (int)value);
		}
		for (auto& [hint, value] : info.strHints)
		{
			glfwWindowHintString((int)hint, value.c_str());
		}

		m_window = glfwCreateWindow(
			info.width
			, info.height
			, info.title.c_str()
			, nullptr
			, contextToShare.m_window);
		if (m_window)
		{
			connectDispatchCallbacks();
		}
	}

	Window::~Window()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);
			m_window = nullptr;
		}
	}

	void Window::connectDispatchCallbacks()
	{
		glfwSetWindowUserPointer(m_window, this);

		glfwSetKeyCallback(m_window, Window::dispatch_key_callback);
		glfwSetCharCallback(m_window, Window::dispatch_char_callback);
		glfwSetCursorPosCallback(m_window, Window::dispatch_cursor_pos_callback);
		glfwSetMouseButtonCallback(m_window, Window::dispatch_mouse_button_callback);
		glfwSetCursorEnterCallback(m_window, Window::dispatch_cursor_enter_callback);
		glfwSetScrollCallback(m_window, Window::dispatch_scroll_callback);
		glfwSetDropCallback(m_window, Window::dispatch_drop_callback);

		glfwSetWindowCloseCallback(m_window, Window::dispatch_window_close_callback);
		glfwSetWindowSizeCallback(m_window, Window::dispatch_window_size_callback);
		glfwSetFramebufferSizeCallback(m_window, Window::dispatch_framebuffer_size_callback);
		glfwSetWindowPosCallback(m_window, Window::dispatch_window_pos_callback);
		glfwSetWindowIconifyCallback(m_window, Window::dispatch_window_iconify_callback);
		glfwSetWindowMaximizeCallback(m_window, Window::dispatch_window_maximize_callback);
		glfwSetWindowFocusCallback(m_window, Window::dispatch_window_focus_callback);
		glfwSetWindowRefreshCallback(m_window, Window::dispatch_window_refresh_callback);
		glfwSetWindowContentScaleCallback(m_window, Window::dispatch_window_content_scale_callback);
	}


	// utilities
	void Window::hide()
	{
		glfwHideWindow(m_window);
	}

	void Window::show()
	{
		glfwShowWindow(m_window);
	}

	void Window::gainFocus()
	{
		glfwFocusWindow(m_window);
	}

	void Window::requestAttention()
	{
		glfwRequestWindowAttention(m_window);
	}

	void Window::makeContextCurrent()
	{
		if (m_window && m_window != glfwGetCurrentContext())
		{
			// TODO : error checks
			glfwMakeContextCurrent(m_window);

			load_opengl_functions();
		}
	}

	void Window::swapBuffers()
	{
		glfwSwapBuffers(m_window);
	}

	void Window::iconify()
	{
		glfwIconifyWindow(m_window);
	}

	void Window::restore()
	{
		glfwRestoreWindow(m_window);
	}

	void Window::maximize()
	{
		glfwMaximizeWindow(m_window);
	}

	void Window::enterFullscreen(bool enter)
	{ 
		if (fullscreen() == enter)
		{
			return;
		}

		if (enter)
		{
			m_posRestore  = position();
			m_sizeRestore = size();

			GLFWmonitor* mainMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* videoMode = glfwGetVideoMode(mainMonitor);

			glfwSetWindowMonitor(
				m_window
				, mainMonitor
				, 0, 0
				, videoMode->width, videoMode->height
				, videoMode->refreshRate);
		}
		else
		{
			glfwSetWindowMonitor(
				m_window
				, nullptr
				, m_posRestore.x, m_posRestore.y
				, m_sizeRestore.width, m_sizeRestore.height
				, 0);
		}
	}

	GLFWwindow* Window::getGLFWwindow() const
	{
		return m_window;
	}


	// properties
	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}

	void Window::shouldClose(bool flag)
	{
		glfwSetWindowShouldClose(m_window, flag);
	}


	Vec2<double> Window::cursorPos() const
	{
		double x, y;
		glfwGetCursorPos(m_window, &x, &y);
		return {x, y};
	}

	void Window::cursorPos(double x, double y)
	{
		glfwSetCursorPos(m_window, x, y);
	}


	Size<int> Window::size() const
	{
		int width{};
		int height{};
		glfwGetWindowSize(m_window, &width, &height);

		return {width, height};
	}

	void Window::size(int width, int height)
	{
		glfwSetWindowSize(m_window, width, height);
	}


	Vec2<int> Window::position() const
	{
		int xPos{};
		int yPos{};
		glfwGetWindowPos(m_window, &xPos, &yPos);
		return {xPos, yPos};
	}

	void Window::position(int xPos, int yPos)
	{
		glfwSetWindowPos(m_window, xPos, yPos);
	}


	float Window::opacity() const
	{
		return glfwGetWindowOpacity(m_window);
	}

	void Window::opacity(float newOpacity)
	{
		glfwSetWindowOpacity(m_window, newOpacity);
	}


	bool Window::resizable() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_RESIZABLE);
	}

	void Window::resizable(bool flag)
	{
		glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, flag);
	}


	bool Window::decorated() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_DECORATED);
	}

	void Window::decorated(bool flag)
	{
		glfwSetWindowAttrib(m_window, GLFW_DECORATED, flag);
	}


	bool Window::floating() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_FLOATING);
	}

	void Window::floating(bool flag)
	{
		glfwSetWindowAttrib(m_window, GLFW_FLOATING, flag);
	}


	bool Window::focusOnShow() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_FOCUS_ON_SHOW);
	}

	void Window::focusOnShow(bool flag)
	{
		glfwSetWindowAttrib(m_window, GLFW_FOCUS_ON_SHOW, flag);
	}


	bool Window::autoIconify() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_AUTO_ICONIFY);
	}

	void Window::autoIconify(bool flag)
	{
		glfwSetWindowAttrib(m_window, GLFW_AUTO_ICONIFY, flag);
	}


	// properties (get)
	Rect<int> Window::frameSize() const
	{
		int left{};
		int top{};
		int right{};
		int bottom{};
		glfwGetWindowFrameSize(m_window, &left, &top ,&right, &bottom);
		return {left, top, right, bottom};
	}

	Size<int> Window::framebufferSize() const
	{
		int width{};
		int height{};
		glfwGetFramebufferSize(m_window, &width, &height);
		return {width, height};
	}

	Vec2<float> Window::contentScale() const
	{
		float xScale{};
		float yScale{};
		glfwGetWindowContentScale(m_window, &xScale, &yScale);
		return {xScale, yScale};
	}

	bool Window::focused() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_FOCUSED);
	}

	bool Window::valid() const
	{
		return m_window != nullptr;
	}

	bool Window::visible() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_VISIBLE);
	}

	bool Window::hovered() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_HOVERED);
	}

	bool Window::transparent() const
	{
		return glfwGetWindowAttrib(m_window, GLFW_TRANSPARENT_FRAMEBUFFER);
	}

	bool Window::fullscreen() const
	{
		return glfwGetWindowMonitor(m_window) != nullptr;
	}

	// properties (set)
	void Window::title(const std::string& newTitle)
	{
		glfwSetWindowTitle(m_window, newTitle.c_str());
	}

	void Window::aspectRatio(int width, int height)
	{
		glfwSetWindowAspectRatio(m_window, width, height);
	}

	void Window::sizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight)
	{
		glfwSetWindowSizeLimits(m_window, minWidth, minHeight, maxWidth, maxHeight);
	}

	// properties (TODO)
	void Window::icon()
	{
		// TODO
	}

	void Window::monitor()
	{
		// TODO
	}

	Value Window::clientAPI()
	{
		return Value{glfwGetWindowAttrib(m_window, GLFW_CONTEXT_CREATION_API)};
	}


	// empty callbacks
	void Window::keyPressEvent(int key, int scancode, int action, int mods)
	{}

	void Window::charEvent(unsigned int codepoint)
	{}

	void Window::mouseMoveEvent(double xpos, double ypos)
	{}

	void Window::mouseButtonEvent(int button, int action, int mods)
	{}

	void Window::mouseEnterEvent(int entered)
	{}

	void Window::scrollEvent(double xOffset, double yOffset)
	{}

	void Window::dropEvent(int pathCount, const char* paths[])
	{}

	void Window::closeEvent()
	{}

	void Window::resizeEvent(int width, int height)
	{}

	void Window::framebufferResizeEvent(int width, int height)
	{}

	void Window::moveEvent(int xPos, int yPos)
	{}

	void Window::minimizeEvent(int minimized)
	{}

	void Window::maximizeEvent(int maximized)
	{}

	void Window::focusEvent(int focused)
	{}

	void Window::refreshEvent()
	{}

	void Window::contentScaleEvent(float xScale, float yScale)
	{}
}

// general functions
namespace glfw
{
	bool initialize()
	{
		return glfwInit();
	}

	void terminate()
	{
		glfwTerminate();
	}


	void set_default_window_hints()
	{
		glfwDefaultWindowHints();
	}


	void poll_events()
	{
		glfwPollEvents();
	}

	void wait_events()
	{
		glfwWaitEvents();
	}

	void wait_events_timeout(float waitSec)
	{
		glfwWaitEventsTimeout(waitSec);
	}

	void post_empty_event()
	{
		glfwPostEmptyEvent();
	}


	void set_swap_interval(int interval)
	{
		glfwSwapInterval(interval);
	}


	Error get_error()
	{
		const char* errorMsg{};
		int errorCode = glfwGetError(&errorMsg);

		return Error{errorCode, errorMsg};
	}

	void set_error_callback(ErrorCallback callback)
	{
		glfwSetErrorCallback(callback);
	}


	double get_time()
	{
		return glfwGetTime();
	}

	void set_time(double t)
	{
		glfwSetTime(t);
	}

	std::uint64_t get_timer_value()
	{
		return glfwGetTimerValue();
	}

	std::uint64_t get_timer_frequency()
	{
		return glfwGetTimerFrequency();
	}
}